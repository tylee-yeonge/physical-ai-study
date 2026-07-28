    @staticmethod
    def forward(
        ctx: torch.autograd.function.FunctionCtx,
        A: torch.Tensor,
        B: torch.Tensor,
        out: Optional[torch.Tensor] = None,
        bias: Optional[torch.Tensor] = None,
        state: Optional[MatmulLtState] = None,
    ):
        state = state or MatmulLtState()

        # default of pytorch behavior if inputs are empty
        ctx.is_empty = False
        if prod(A.shape) == 0:
            ctx.is_empty = True
            ctx.A = A
            ctx.B = B
            ctx.bias = bias
            if A.shape[-1] == B.shape[0]:
                return torch.empty(A.shape[:-1] + B.shape[1:], dtype=A.dtype, device=A.device)
            else:
                return torch.empty(A.shape[:-1] + B.shape[:1], dtype=A.dtype, device=A.device)

        input_shape = A.shape

        # Cast A to fp16
        if A.dtype != torch.float16 and not _is_compiling():
            warnings.warn(f"MatMul8bitLt: inputs will be cast from {A.dtype} to float16 during quantization")

        if len(A.shape) == 3:
            A = A.reshape(-1, A.shape[-1])

        # 1. Quantize A. Note that as a side-effect, outliers are suppressed in CA/CAt.
        if ctx.needs_input_grad[1]:
            # Slower path
            CA, CAt, SCA, SCAt, outlier_cols = F.int8_double_quant(A.to(torch.float16), threshold=state.threshold)
        else:
            # Fast path
            CA, SCA, outlier_cols = F.int8_vectorwise_quant(A.to(torch.float16), threshold=state.threshold)
            CAt = SCAt = None

        has_grad = False

        if state.has_fp16_weights or state.CB is None:
            has_grad = getattr(B, "grad", None) is not None
            is_transposed = not B.is_contiguous() and B.shape[0] == B.stride(1)
            if is_transposed:
                B = B.contiguous()

            if (state.is_training and not has_grad) or state.CB is None or state.SCB is None:
                state.reset_grads()

                # 2. Quantize B
                state.CB, state.SCB, _ = F.int8_vectorwise_quant(B.to(torch.float16))

        # Handle sparse decomposition
        if state.threshold > 0.0:
            state.idx = outlier_cols

            # Mixed Int8 Matmul + Dequant + Bias
            output, subA = torch.ops.bitsandbytes.int8_mixed_scaled_mm(
                A,
                CA,
                state.CB,
                SCA,
                state.SCB,
                outlier_cols,
                bias,
            )

        else:
            # Int8 Matmul + Dequant + Bias
            output = torch.ops.bitsandbytes.int8_scaled_mm.default(
                CA, state.CB, SCA, state.SCB, bias=bias, dtype=A.dtype
            )
            subA = None

        # 5. Save state
        ctx.state = state

        ctx.grad_shape = input_shape
        ctx.dtype_A = A.dtype
        ctx.dtype_bias = None if bias is None else bias.dtype

        if any(ctx.needs_input_grad[:2]):
            ctx.tensors = (CAt, subA, A)
            ctx.tensor_states = (SCAt, state.idx)
        else:
            ctx.tensors = [None, None, None]
            ctx.tensor_states = (None, None)
            ctx.save_for_backward(None, None)

        output_shape = (*input_shape[:-1], state.CB.shape[0])

        if len(input_shape) == 3:
            return output.reshape(output_shape)

        return output

def matmul_4bit(
    A: torch.Tensor,
    B: torch.Tensor,
    quant_state: F.QuantState,
    out: Optional[torch.Tensor] = None,
    bias: Optional[torch.Tensor] = None,
):
    assert quant_state is not None
    # Change dtype to input dtype on CPU
    if A.device.type == "cpu":
        quant_state.dtype = A.dtype

        if getattr(quant_state, "packing_format_for_cpu", False):
            out = F.gemv_4bit(A, B, out, state=quant_state)
            if bias is not None:
                out += bias
            return out
        else:
            return MatMul4Bit.apply(A, B, out, bias, quant_state)

    if A.numel() == A.shape[-1] and A.requires_grad == False and A.device.type != "hpu":
        if A.shape[-1] % quant_state.blocksize != 0:
            warn(
                f"Some matrices hidden dimension is not a multiple of {quant_state.blocksize} and efficient inference kernels are not supported for these (slow). Matrix input size found: {A.shape}",
            )
            return MatMul4Bit.apply(A, B, out, bias, quant_state)
        else:
            out = F.gemv_4bit(A, B.t(), out, state=quant_state)
            if bias is not None:
                out += bias
            return out
    else:
        return MatMul4Bit.apply(A, B, out, bias, quant_state)

@staticmethod
def forward(ctx, A, B, out=None, bias=None, quant_state: Optional[F.QuantState] = None):
    # default of pytorch behavior if inputs are empty
    ctx.is_empty = False
    if prod(A.shape) == 0:
        ctx.is_empty = True
        ctx.A = A
        ctx.B = B
        ctx.bias = bias
        B_shape = quant_state.shape
        if A.shape[-1] == B_shape[0]:
            return torch.empty(A.shape[:-1] + B_shape[1:], dtype=A.dtype, device=A.device)
        else:
            return torch.empty(A.shape[:-1] + B_shape[:1], dtype=A.dtype, device=A.device)

    # 1. Dequantize
    # 2. MatmulnN
    output = torch.nn.functional.linear(A, F.dequantize_4bit(B, quant_state).to(A.dtype).t(), bias)

    # 3. Save state
    ctx.state = quant_state
    ctx.dtype_A, ctx.dtype_B, ctx.dtype_bias = A.dtype, B.dtype, None if bias is None else bias.dtype

    if any(ctx.needs_input_grad[:2]):
        ctx.tensors = (None, B)
    else:
        ctx.tensors = (None, None)

    return output