# Week 1 실습: PyTorch 기초 재정비


> **실습 목표**: CUDA 환경 세팅 → Tensor 연산 → CNN 학습 → ResNet Fine-tuning → TensorBoard → Checkpoint
> **예상 시간**: 6-8시간


---


## 환경 설정


```bash
cd Studies/Phase\ 3/week1

# 가상환경 생성 및 의존성 설치 (apt 패키지 설치 포함, root 권한 필요)
# 내부에서 .venv-week1 생성 후 requirements.txt 설치
./pip_install.sh

# 가상환경 활성화 (sh 종료 후 현재 shell에서 직접 활성화 필요)
source .venv-week1/bin/activate


# GPU 확인
python -c "import torch; print(f'CUDA: {torch.cuda.is_available()}, Device: {torch.cuda.get_device_name(0) if torch.cuda.is_available() else \"CPU\"}')"
```


---


## 실습 1: Tensor 기초 연산

이 실습은 모델을 학습시키는 것이 아니라, 이후 모든 실습에서 쓸 PyTorch의 기본 자료구조인 Tensor를 손에 익히는 준비 운동이다.

### 왜 Tensor부터 시작하나

딥러닝의 모든 데이터(이미지, 라벨, 가중치, gradient)는 Tensor로 표현된다. 모델 학습은 결국 Tensor 연산의 연속이다. 연산과 shape 변환, GPU 이동이 손에 안 익으면 이후 실습에서 디버깅의 대부분이 "shape이 안 맞아서" 막힌다. NumPy를 이미 안다면 90%는 같지만, 두 가지가 결정적으로 다르다. (1) GPU로 옮길 수 있고(`.to(device)`), (2) 연산 기록을 추적해 자동 미분이 된다(실습 2). 이 두 가지가 NumPy 대신 Tensor를 쓰는 이유다.

### 학습 목표

| 목표 | 코드에서 해당 부분 | 무엇을 익히나 |
|------|------------------|--------------|
| shape 다루기 | `view`/`reshape`/`permute`/`squeeze` | 텐서 모양을 자유자재로 바꾼다. 에러의 대부분이 shape 불일치다 |
| GPU 이동 | `x.to(device)` | 연산을 GPU에서 수행한다. CPU 텐서와 GPU 텐서는 섞어서 연산할 수 없다 |
| NumPy 상호변환 | `from_numpy`/`.numpy()` | OpenCV 등 외부 데이터와 텐서를 오간다 |
| 이미지 텐서 변환 | 1-6: HWC uint8 BGR -> CHW float [0-1] | OpenCV와 PyTorch의 형식 차이. 이후 모든 비전 실습의 전처리 기본이다 |

### 핵심 포인트

- 1-6 이미지 변환이 이번 실습의 실전 핵심이다. OpenCV는 `(H, W, C)`/uint8/BGR, PyTorch는 `(C, H, W)`/float/0-1을 쓴다. 이 변환을 못 하면 모델 입력이 깨진다.
- `view`는 메모리가 연속일 때만 동작하고 `reshape`는 필요하면 복사한다. `permute` 직후 `view`가 실패하는 함정이 여기서 나온다.


**파일명**: `practice_tensor.py`


```python
"""
실습 1: PyTorch Tensor 기초 연산
목표: 텐서 생성, 연산, shape 변환, GPU 이동을 자유자재로 다룬다.
"""
import torch
import numpy as np


print("=" * 50)
print("실습 1: Tensor 기초 연산")
print("=" * 50)


# -- 1-1. 텐서 생성 --
print("\n[1-1] 텐서 생성")
zeros = torch.zeros(3, 4) # 0으로 채운 3x4 텐서 (행 3, 열 4)
ones = torch.ones(2, 3) # 1로 채운 2x3 텐서
rand = torch.randn(3, 3) # 표준정규분포
arange = torch.arange(0, 10, 2) # 0부터 10 직전까지 2씩 증가: [0, 2, 4, 6, 8]
linspace = torch.linspace(0, 1, 5) # 0과 1 포함, 균등 간격 5개: [0, 0.25, 0.5, 0.75, 1]


print(f"zeros shape: {zeros.shape}")
print(f"randn:\n{rand}")
print(f"arange: {arange}")
print(f"linspace: {linspace}")


# -- 1-2. NumPy 변환 --
print("\n[1-2] NumPy 변환")
np_array = np.array([[1, 2], [3, 4]], dtype=np.float32) # NumPy 2x2 배열 생성
tensor_from_np = torch.from_numpy(np_array) # NumPy -> 텐서 (메모리 공유, 복사 없음)
np_from_tensor = tensor_from_np.numpy() # 텐서 -> NumPy (역방향 변환)
print(f"NumPy → Tensor: {tensor_from_np}")
print(f"Tensor → NumPy: {np_from_tensor}")


# -- 1-3. 연산 --
print("\n[1-3] 연산")
a = torch.tensor([[1.0, 2.0], [3.0, 4.0]]) # 값을 직접 지정한 2x2 텐서
b = torch.tensor([[5.0, 6.0], [7.0, 8.0]])


print(f"a + b = {a + b}") # 같은 위치 원소끼리 덧셈
print(f"a * b (요소별) = {a * b}") # 같은 위치 원소끼리 곱셈 (행렬곱 아님)
print(f"a @ b (행렬곱) = \n{a @ b}") # 행렬 곱셈 (matmul)
print(f"a.sum() = {a.sum()}") # 모든 원소의 합
print(f"a.mean() = {a.mean()}") # 모든 원소의 평균
print(f"a.max() = {a.max()}") # 가장 큰 원소 하나


# -- 1-4. Shape 변환 --
print("\n[1-4] Shape 변환")
x = torch.randn(2, 3, 4) # 3차원 텐서 (원소 2*3*4 = 24개)
print(f"원래 shape: {x.shape}")
print(f"view(2, 12): {x.view(2, 12).shape}") # 원소 수 유지하며 모양 변경 (메모리 연속 필요)
print(f"reshape(6, 4): {x.reshape(6, 4).shape}") # view와 비슷하나 필요 시 복사 (더 안전)
print(f"permute(2,0,1): {x.permute(2, 0, 1).shape}") # 축 순서 변경 [2,3,4] -> [4,2,3]
print(f"unsqueeze(0): {x.unsqueeze(0).shape}") # 맨 앞에 크기 1 차원 추가
print(f"squeeze 후: {x.unsqueeze(0).squeeze(0).shape}") # 크기 1 차원 제거 (unsqueeze의 반대)


# -- 1-5. GPU 이동 --
print("\n[1-5] GPU 이동")
device = torch.device('cuda' if torch.cuda.is_available() else 'cpu') # GPU 있으면 cuda, 없으면 cpu
print(f"사용 디바이스: {device}")
x_gpu = x.to(device) # 텐서를 선택한 디바이스로 이동 (GPU면 GPU 메모리로 복사)
print(f"텐서 디바이스: {x_gpu.device}")


# -- 1-6. 이미지 텐서 변환 연습 --
print("\n[1-6] 이미지 텐서 변환")
# OpenCV 형식: (H, W, C), uint8, BGR
fake_img = np.random.randint(0, 255, (480, 640, 3), dtype=np.uint8)
# PyTorch 형식: (C, H, W), float32, [0, 1]
img_tensor = torch.from_numpy(fake_img).permute(2, 0, 1).float() / 255.0 # HWC->CHW, float 변환, 255로 나눠 0-1 정규화
print(f"OpenCV shape: {fake_img.shape} → PyTorch shape: {img_tensor.shape}")
print(f"dtype: {img_tensor.dtype}, range: [{img_tensor.min():.2f}, {img_tensor.max():.2f}]")


print("\n 실습 1 완료!")
```


**실행**:
```bash
python practice_tensor.py
```


**예상 출력**:
```
==================================================
실습 1: Tensor 기초 연산
==================================================


[1-1] 텐서 생성
  zeros shape: torch.Size([3, 4])
  ...
[1-5] GPU 이동
  사용 디바이스: cuda
  텐서 디바이스: cuda:0


- 실습 1 완료!
```


---


## 실습 2: autograd 자동 미분

autograd는 PyTorch가 "학습"을 가능하게 하는 엔진이다. 이 실습은 모델 없이, 자동 미분이 어떻게 동작하는지 가장 작은 예제로 직접 확인한다.

### 왜 이걸 따로 보나

딥러닝 학습은 loss를 파라미터로 미분해서(gradient) 그 반대 방향으로 조금씩 이동하는 것(경사 하강)이다. 파라미터가 수백만 개인 모델에서 미분을 손으로 할 수 없으니, PyTorch가 순전파 연산을 기록해두고 `backward()` 한 번에 자동으로 계산해준다. 이것이 autograd다. 실습 3 이후의 `loss.backward()`/`optimizer.step()`은 전부 이 위에서 돈다. 여기서 원리를 보지 않으면 이후 "왜 grad를 `zero_()` 하는지", "왜 `no_grad`를 쓰는지"가 마법처럼 보인다.

### 학습 목표

| 목표 | 코드에서 해당 부분 | 무엇을 익히나 |
|------|------------------|--------------|
| 미분 추적 3단계 | 2-1/2-2: `requires_grad` -> `backward()` -> `.grad` | 추적을 켜고, 역전파하고, gradient가 저장되는 흐름 |
| 경사 하강 직접 구현 | 2-3: `w -= lr * w.grad` | optimizer 없이 학습 루프의 뼈대를 손으로 짠다 |
| 추적 끄기 | 2-4: `with torch.no_grad()` | 추론/평가에서는 미분 추적을 꺼 메모리와 속도를 아낀다 |

### 핵심 포인트

- 2-3에서 손으로 짠 "예측 -> loss -> backward -> 업데이트 -> grad 초기화" 5단계는 실습 3의 optimizer가 자동으로 해주는 것과 정확히 같다. `grad.zero_()`를 빼면 gradient가 누적돼 학습이 망가지는 것을 직접 확인하는 게 포인트다.
- 학습된 `w`가 정답 2.0, `b`가 1.0으로 수렴하는 것을 보며 "경사 하강이 실제로 정답을 찾는다"를 눈으로 확인한다.


**파일명**: `practice_autograd.py`


```python
"""
실습 2: autograd 자동 미분
목표: requires_grad, backward, grad의 동작을 이해한다.
"""
import torch


print("=" * 50)
print("실습 2: autograd 자동 미분")
print("=" * 50)


# -- 2-1. 기본 자동 미분 --
print("\n[2-1] 기본 자동 미분")
x = torch.tensor(3.0, requires_grad=True) # requires_grad=True -> 이 텐서의 미분을 추적
y = x ** 2 + 2 * x + 1 # y = x^2 + 2x + 1
y.backward() # 역전파: dy/dx를 계산해 x.grad에 저장
print(f"x = {x.item()}")
print(f"y = x^2 + 2x + 1 = {y.item()}")
print(f"dy/dx = 2x + 2 = {x.grad.item()}") # 2*3 + 2 = 8


# -- 2-2. 벡터 입력 --
print("\n[2-2] 벡터 입력")
x = torch.tensor([1.0, 2.0, 3.0], requires_grad=True) # 벡터 텐서, 미분 추적
y = (x ** 2).sum() # y = x1^2+x2^2+x3^2 (backward는 스칼라만 가능 -> sum으로 합침)
y.backward() # 각 원소에 대한 미분을 x.grad에 저장
print(f"x = {x.data}")
print(f"y = sum(x^2) = {y.item()}")
print(f"dy/dx = 2x = {x.grad}") # [2, 4, 6]


# -- 2-3. 간단한 선형 회귀 --
print("\n[2-3] 수동 선형 회귀 (y = 2x + 1)")
torch.manual_seed(42) # 랜덤 시드 고정 (실행할 때마다 같은 결과)


# 데이터 생성: y = 2x + 1 + noise
X = torch.randn(100, 1) # 입력 데이터 100개 (정규분포 난수)
y_true = 2 * X + 1 + 0.1 * torch.randn(100, 1) # 정답 = 2x+1에 약간의 노이즈 추가


# 학습할 파라미터
w = torch.randn(1, requires_grad=True) # 학습할 가중치, 난수로 초기화
b = torch.zeros(1, requires_grad=True) # 학습할 편향, 0으로 초기화
lr = 0.1 # 학습률 (한 스텝에 얼마나 업데이트할지)


for epoch in range(100):
    # 순전파
    y_pred = X * w + b # 현재 w, b로 예측값 계산
    loss = ((y_pred - y_true) ** 2).mean() # MSE: 예측-정답 차이의 제곱 평균


    # 역전파
    loss.backward() # dloss/dw, dloss/db를 계산


    # 파라미터 업데이트 (gradient descent)
    with torch.no_grad(): # 업데이트 연산 자체는 미분 추적에서 제외
        w -= lr * w.grad # gradient 반대 방향으로 w 이동 (경사 하강)
        b -= lr * b.grad


    # gradient 초기화
    w.grad.zero_() # 다음 epoch을 위해 grad 비우기 (안 하면 누적됨)
    b.grad.zero_()


    if (epoch + 1) % 20 == 0:
        print(f"Epoch {epoch+1:3d}: loss={loss.item():.4f}, w={w.item():.4f}, b={b.item():.4f}")


print(f"\n 최종: w={w.item():.4f} (정답: 2.0), b={b.item():.4f} (정답: 1.0)")


# -- 2-4. no_grad 블록 --
print("\n[2-4] torch.no_grad() 사용")
x = torch.randn(3, requires_grad=True) # 미분 추적이 켜진 텐서
print(f"requires_grad 상태: {x.requires_grad}")


with torch.no_grad(): # 이 블록 안 연산은 미분 그래프를 만들지 않음 (추론/평가용)
    y = x * 2
    print(f"no_grad 블록 내 y.requires_grad: {y.requires_grad}") # x는 추적 중이지만 y는 False


print("\n 실습 2 완료!")
```


**실행**:
```bash
python practice_autograd.py
```


---


## 실습 3: CIFAR-10 CNN 학습

앞 두 실습(텐서, autograd)을 합쳐 처음으로 진짜 모델을 처음부터 학습시킨다. 이미지 분류의 전체 파이프라인(데이터 -> 모델 -> 학습 루프 -> 평가 -> 로깅 -> 체크포인트)을 한 번에 경험하는 것이 목표다.

### 왜 직접 CNN을 짜나

지금까지는 조각(텐서, 미분)이었다. 실무에서 쓰는 학습은 이 조각들이 정해진 골격으로 조립된다. Dataset/DataLoader가 배치를 공급하고, 모델이 순전파하고, criterion이 loss를 내고, optimizer가 backward와 step을 한다. 이 골격은 이후 YOLO 학습(week4)까지 그대로 반복된다. Ultralytics 같은 고수준 API는 이 골격을 숨기기 때문에, 한 번은 손으로 짜봐야 API가 내부에서 무엇을 하는지 안다.

### 학습 목표

| 목표 | 코드에서 해당 부분 | 무엇을 익히나 |
|------|------------------|--------------|
| 데이터 공급 | `CIFAR10` + `transform` + `DataLoader` | 데이터를 배치로 공급하고, 증강은 학습셋에만 적용 |
| CNN 설계 | `Conv-BN-ReLU-Pool` 3블록 + FC | 특징 추출부와 분류부의 분리 |
| 학습/평가 분리 | `train()`/`eval()`, `no_grad` | BatchNorm이 모드에 따라 다르게 동작 |
| 로깅/체크포인트 | TensorBoard, `best_acc` 저장 | 학습 과정 추적과 최고 모델 보존 |

### 핵심 포인트

- 학습 transform에만 증강(flip/crop)을 넣고 테스트에는 넣지 않는다. 평가는 고정된 조건이어야 공정하다.
- `model.train()`과 `eval()`을 구분하는 이유는 BatchNorm/Dropout이 모드별로 다르게 동작하기 때문이다. 추론할 때 train 모드면 결과가 흔들린다.
- `val_acc`가 최고일 때만 저장한다(best). 마지막 epoch이 최고가 아닐 수 있다(과적합).


**파일명**: `practice_cifar10.py`


```python
"""
실습 3: CIFAR-10 CNN 학습
목표: Dataset, DataLoader, 학습 루프, 평가를 직접 구현한다.
"""
import os # 파일/디렉토리 작업
import torch
import torch.nn as nn # 신경망 레이어 (Conv, Linear, BatchNorm 등)
import torch.optim as optim # 옵티마이저 (Adam, SGD 등)
import torchvision # 이미지 데이터셋/모델 모음
import torchvision.transforms as transforms # 이미지 전처리/증강
from torch.utils.data import DataLoader # 배치 단위 데이터 공급
from torch.utils.tensorboard import SummaryWriter # 학습 곡선 로깅
import time


print("=" * 50)
print("실습 3: CIFAR-10 CNN 학습")
print("=" * 50)


# -- 설정 --
device = torch.device('cuda' if torch.cuda.is_available() else 'cpu') # GPU 있으면 GPU 사용
print(f"디바이스: {device}")
os.makedirs('outputs', exist_ok=True) # 결과물 폴더 (체크포인트/로그를 수업 자료와 분리)


# -- 데이터 준비 --
transform_train = transforms.Compose([ # 학습용 전처리 (데이터 증강 포함)
    transforms.RandomHorizontalFlip(), # 50% 확률로 좌우 반전
    transforms.RandomCrop(32, padding=4), # 4px 패딩 후 32x32 무작위 크롭
    transforms.ToTensor(), # PIL 이미지 -> 텐서 (CHW, 0-1)
    transforms.Normalize((0.4914, 0.4822, 0.4465), # 채널별 평균/표준편차로 정규화
                         (0.2470, 0.2435, 0.2616)),
])


transform_test = transforms.Compose([ # 테스트용 전처리 (증강 없이 정규화만)
    transforms.ToTensor(),
    transforms.Normalize((0.4914, 0.4822, 0.4465),
                         (0.2470, 0.2435, 0.2616)),
])


trainset = torchvision.datasets.CIFAR10( # CIFAR-10 학습셋 50000장 (없으면 다운로드)
    root='./data', train=True, download=True, transform=transform_train)
testset = torchvision.datasets.CIFAR10( # CIFAR-10 테스트셋 10000장
    root='./data', train=False, download=True, transform=transform_test)


trainloader = DataLoader(trainset, batch_size=128, shuffle=True, num_workers=2) # 128장씩, 매 epoch 섞음
testloader = DataLoader(testset, batch_size=128, shuffle=False, num_workers=2) # 평가용, 순서 고정


classes = ('airplane', 'automobile', 'bird', 'cat', 'deer', # 클래스 인덱스 -> 이름
           'dog', 'frog', 'horse', 'ship', 'truck')


print(f"학습 데이터: {len(trainset)}장, 테스트 데이터: {len(testset)}장")


# -- CNN 모델 --
class SimpleCNN(nn.Module): # 3단 컨볼루션 블록 + 분류기로 구성된 CNN
    def __init__(self): # 학습할 레이어들을 등록
        super().__init__()
        self.features = nn.Sequential( # 특징 추출부 (이미지 -> 특징 맵)
            nn.Conv2d(3, 32, 3, padding=1), # 3채널 -> 32채널, 3x3 커널, padding으로 크기 유지
            nn.BatchNorm2d(32), # 배치 정규화 (학습 안정화)
            nn.ReLU(), # 활성화 함수 (음수를 0으로)
            nn.MaxPool2d(2), # 2x2 최댓값 풀링 (가로세로 절반: 32->16)


            nn.Conv2d(32, 64, 3, padding=1), # 32채널 -> 64채널
            nn.BatchNorm2d(64),
            nn.ReLU(),
            nn.MaxPool2d(2), # 크기 절반 (16->8)


            nn.Conv2d(64, 128, 3, padding=1), # 64채널 -> 128채널
            nn.BatchNorm2d(128),
            nn.ReLU(),
            nn.AdaptiveAvgPool2d(1), # 공간 차원을 1x1로 평균 (Global Average Pooling)
        )
        self.classifier = nn.Linear(128, 10) # 분류부: 128차원 특징 -> 10개 클래스 점수


    def forward(self, x): # 입력 x가 레이어를 통과하는 순서 정의
        x = self.features(x) # 특징 추출 -> [B, 128, 1, 1]
        x = x.view(x.size(0), -1) # [B, 128, 1, 1] -> [B, 128]로 평탄화
        x = self.classifier(x) # [B, 10] 클래스별 점수
        return x


model = SimpleCNN().to(device) # 모델 생성 후 디바이스로 이동
criterion = nn.CrossEntropyLoss() # 손실 함수: 분류용 교차 엔트로피
optimizer = optim.Adam(model.parameters(), lr=0.001) # Adam 옵티마이저, 학습률 0.001
scheduler = optim.lr_scheduler.StepLR(optimizer, step_size=15, gamma=0.5) # 15 epoch마다 lr 0.5배


# -- TensorBoard --
writer = SummaryWriter('outputs/runs/cifar10_simple_cnn') # 학습 로그 저장 경로


# -- 학습 함수 --
def train_one_epoch(model, loader, criterion, optimizer, device): # 한 epoch 학습
    model.train() # 학습 모드 (BatchNorm/Dropout 활성화)
    running_loss = 0.0 # 손실 누적용
    correct = 0 # 맞춘 개수 누적용
    total = 0 # 전체 샘플 수 누적용


    for images, labels in loader: # 배치 단위로 순회
        images, labels = images.to(device), labels.to(device) # 데이터를 디바이스로 이동


        optimizer.zero_grad() # 이전 gradient 비우기
        outputs = model(images) # 순전파 -> 클래스 점수
        loss = criterion(outputs, labels) # 손실 계산
        loss.backward() # 역전파 -> gradient 계산
        optimizer.step() # gradient로 가중치 업데이트


        running_loss += loss.item() # 배치 손실 누적
        _, predicted = outputs.max(1) # 점수가 가장 높은 클래스를 예측값으로
        total += labels.size(0) # 배치 크기만큼 전체 수 증가
        correct += predicted.eq(labels).sum().item() # 정답과 일치하는 개수 누적


    return running_loss / len(loader), 100. * correct / total # 평균 손실, 정확도(%)


# -- 평가 함수 --
@torch.no_grad() # 평가 중에는 gradient 계산 안 함 (메모리 절약, 속도 향상)
def evaluate(model, loader, criterion, device): # 테스트셋으로 평가
    model.eval() # 평가 모드 (BatchNorm은 학습 때 누적한 통계 사용)
    running_loss = 0.0
    correct = 0
    total = 0


    for images, labels in loader:
        images, labels = images.to(device), labels.to(device)
        outputs = model(images) # 순전파만 (backward/step 없음)
        loss = criterion(outputs, labels)


        running_loss += loss.item()
        _, predicted = outputs.max(1) # 예측 클래스
        total += labels.size(0)
        correct += predicted.eq(labels).sum().item()


    return running_loss / len(loader), 100. * correct / total # 평균 손실, 정확도(%)


# -- 학습 루프 --
num_epochs = 30 # 전체 데이터를 30번 반복 학습
best_acc = 0.0 # 최고 검증 정확도 기록 (체크포인트 저장 기준)


print(f"\n{'Epoch':>6} | {'Train Loss':>10} | {'Train Acc':>9} | {'Val Loss':>8} | {'Val Acc':>7} | {'Time':>6}")
print("-" * 65)


for epoch in range(num_epochs):
    start = time.time() # epoch 시작 시각 기록


    train_loss, train_acc = train_one_epoch( # 한 epoch 학습
        model, trainloader, criterion, optimizer, device)
    val_loss, val_acc = evaluate( # 테스트셋으로 평가
        model, testloader, criterion, device)


    scheduler.step() # 학습률 스케줄 갱신
    elapsed = time.time() - start # epoch 소요 시간


    # TensorBoard 로깅
    writer.add_scalar('Loss/train', train_loss, epoch)
    writer.add_scalar('Loss/val', val_loss, epoch)
    writer.add_scalar('Accuracy/train', train_acc, epoch)
    writer.add_scalar('Accuracy/val', val_acc, epoch)
    writer.add_scalar('LR', optimizer.param_groups[0]['lr'], epoch)


    # Checkpoint 저장
    if val_acc > best_acc: # 최고 기록 갱신 시에만 저장
        best_acc = val_acc
        torch.save({ # 학습 재개에 필요한 상태를 함께 저장
            'epoch': epoch, # 몇 번째 epoch인지
            'model_state_dict': model.state_dict(), # 모델 가중치
            'optimizer_state_dict': optimizer.state_dict(), # 옵티마이저 상태
            'best_acc': best_acc, # 그때의 정확도
        }, 'outputs/best_cifar10_cnn.pth')
        marker = ' *'
    else:
        marker = ''


    print(f"{epoch+1:>6} | {train_loss:>10.4f} | {train_acc:>8.2f}% | "
          f"{val_loss:>8.4f} | {val_acc:>6.2f}% | {elapsed:>5.1f}s{marker}")


writer.close()
print(f"\n최고 검증 정확도: {best_acc:.2f}%")
print("TensorBoard 확인: tensorboard --logdir=outputs/runs")
print("\n 실습 3 완료!")
```


**실행**:
```bash
python practice_cifar10.py


# TensorBoard 확인
tensorboard --logdir=outputs/runs
```


**예상 결과**:
- SimpleCNN: 약 75-80% 정확도


---


## 실습 4: ResNet-18 Fine-tuning

실습 3은 모델을 0부터 학습했다. 이 실습은 ImageNet으로 이미 학습된 ResNet-18을 가져와 CIFAR-10에 맞게 재활용한다. 이것이 전이 학습(transfer learning)이다.

### 왜 전이 학습을 하나

실무에서 모델을 0부터 학습하는 경우는 드물다. 데이터도 시간도 부족하다. 대신 대규모 데이터(ImageNet 약 120만 장)로 학습된 모델의 특징 추출 능력을 그대로 빌려오고, 마지막 분류층만 내 문제에 맞게 바꾼다. 적은 데이터로도 빠르게 높은 정확도에 도달한다(실습 3의 75%대 vs 이 실습의 90%대). 이 패턴은 Phase 3 내내 반복된다. week4의 YOLO도 COCO 사전학습 가중치에서 출발한다. 여기서 freeze/unfreeze와 작은 학습률의 이유를 익혀두면 그대로 적용된다.

### 학습 목표

| 목표 | 코드에서 해당 부분 | 무엇을 익히나 |
|------|------------------|--------------|
| 마지막 층 교체 | `model.fc = nn.Linear(512, 10)` | 사전학습 구조를 재사용하고 출력만 내 문제에 맞춘다 |
| 2단계 학습 | Phase 1 FC만 / Phase 2 전체 | freeze로 새 FC를 먼저 안정화한 뒤 전체를 미세조정 |
| 작은 학습률 | unfreeze 후 `lr=0.0001` | 잘 학습된 사전학습 가중치를 크게 망가뜨리지 않도록 |

### 핵심 포인트

- `requires_grad=False`가 freeze다. gradient를 만들지 않아 그 층은 변하지 않는다. 처음부터 전체를 큰 학습률로 학습하면 무작위로 초기화된 FC의 큰 gradient가 잘 학습된 backbone을 망가뜨린다. 그래서 FC를 먼저(Phase 1), 그다음 전체를 작은 학습률로(Phase 2) 학습한다.
- 입력을 224로 Resize하는 이유는 ResNet이 ImageNet 224x224로 학습돼 그 크기를 기대하기 때문이다.


**파일명**: `practice_resnet18.py`


```python
"""
실습 4: ResNet-18 Fine-tuning
목표: Pretrained 모델을 CIFAR-10에 맞게 수정하고 학습한다.
"""
import os # 파일/디렉토리 작업
import torch
import torch.nn as nn # 신경망 레이어
import torch.optim as optim # 옵티마이저
import torchvision # 이미지 데이터셋/모델 모음
import torchvision.transforms as transforms # 이미지 전처리/증강
from torch.utils.data import DataLoader # 배치 단위 데이터 공급
from torch.utils.tensorboard import SummaryWriter # 학습 곡선 로깅
import torchvision.models as models # 사전학습된 모델 (ResNet 등)
import time


print("=" * 50)
print("실습 4: ResNet-18 Fine-tuning")
print("=" * 50)


device = torch.device('cuda' if torch.cuda.is_available() else 'cpu') # GPU 있으면 GPU 사용
os.makedirs('outputs', exist_ok=True) # 결과물 폴더 (체크포인트/로그를 수업 자료와 분리)


# -- 데이터 (ResNet용 224x224) --
transform_train = transforms.Compose([ # 학습용 전처리
    transforms.Resize(224), # ResNet은 224x224 입력 기대 -> 32x32를 확대
    transforms.RandomHorizontalFlip(), # 50% 확률 좌우 반전
    transforms.ToTensor(), # PIL 이미지 -> 텐서 (CHW, 0-1)
    transforms.Normalize((0.4914, 0.4822, 0.4465), # 채널별 정규화
                         (0.2470, 0.2435, 0.2616)),
])


transform_test = transforms.Compose([ # 테스트용 전처리 (증강 없음)
    transforms.Resize(224),
    transforms.ToTensor(),
    transforms.Normalize((0.4914, 0.4822, 0.4465),
                         (0.2470, 0.2435, 0.2616)),
])


trainset = torchvision.datasets.CIFAR10( # 학습셋 50000장
    root='./data', train=True, download=True, transform=transform_train)
testset = torchvision.datasets.CIFAR10( # 테스트셋 10000장
    root='./data', train=False, download=True, transform=transform_test)


trainloader = DataLoader(trainset, batch_size=64, shuffle=True, num_workers=2) # 224x224라 batch는 64로 축소
testloader = DataLoader(testset, batch_size=64, shuffle=False, num_workers=2) # 평가용, 순서 고정


# -- ResNet-18 모델 (Pretrained) --
model = models.resnet18(weights='IMAGENET1K_V1') # ImageNet으로 사전학습된 ResNet-18 로드


# 마지막 FC 레이어를 CIFAR-10용으로 교체
num_features = model.fc.in_features # 기존 FC의 입력 차원 (512)
model.fc = nn.Linear(num_features, 10) # 1000클래스 -> 10클래스 출력으로 교체


model = model.to(device)


# -- Backbone freeze (선택사항) --
# 처음 몇 에폭은 FC만 학습 → 이후 전체 학습
for param in model.parameters():
    param.requires_grad = False # 전체 레이어 freeze (미분 추적 끔 -> 학습 안 됨)
for param in model.fc.parameters():
    param.requires_grad = True # FC 레이어만 다시 학습 가능하게


criterion = nn.CrossEntropyLoss() # 손실 함수: 분류용 교차 엔트로피
optimizer = optim.Adam(model.fc.parameters(), lr=0.001) # FC 파라미터만 최적화 대상


writer = SummaryWriter('outputs/runs/cifar10_resnet18')


# -- Phase 1: FC만 학습 (5 epochs) --
print("\n-- Phase 1: FC 레이어만 학습 (Backbone Frozen) --")
for epoch in range(5): # FC만 5 epoch 학습
    model.train() # 학습 모드
    running_loss = 0.0
    correct = 0
    total = 0


    for images, labels in trainloader:
        images, labels = images.to(device), labels.to(device)
        optimizer.zero_grad() # 이전 gradient 비우기
        outputs = model(images) # 순전파
        loss = criterion(outputs, labels) # 손실 계산
        loss.backward() # 역전파
        optimizer.step() # 가중치 업데이트 (FC만)


        running_loss += loss.item()
        _, predicted = outputs.max(1) # 예측 클래스
        total += labels.size(0)
        correct += predicted.eq(labels).sum().item() # 맞춘 개수 누적


    acc = 100. * correct / total # 학습 정확도(%)
    print(f"Epoch {epoch+1}: Loss={running_loss/len(trainloader):.4f}, Acc={acc:.2f}%")


# -- Phase 2: 전체 모델 학습 (Unfreeze) --
print("\n-- Phase 2: 전체 모델 학습 (Backbone Unfrozen) --")
for param in model.parameters():
    param.requires_grad = True # 전체 레이어 unfreeze (이제 backbone도 학습)


optimizer = optim.Adam(model.parameters(), lr=0.0001) # 작은 lr (사전학습 가중치를 크게 망가뜨리지 않도록)
scheduler = optim.lr_scheduler.CosineAnnealingLR(optimizer, T_max=15) # lr을 코사인 곡선으로 점차 감소


best_acc = 0.0 # 최고 검증 정확도 기록
for epoch in range(15): # 전체 모델 15 epoch 학습
    model.train() # 학습 모드
    running_loss = 0.0
    correct = 0
    total = 0


    for images, labels in trainloader:
        images, labels = images.to(device), labels.to(device)
        optimizer.zero_grad() # gradient 초기화
        outputs = model(images) # 순전파
        loss = criterion(outputs, labels) # 손실 계산
        loss.backward() # 역전파
        optimizer.step() # 가중치 업데이트 (전체)


        running_loss += loss.item()
        _, predicted = outputs.max(1)
        total += labels.size(0)
        correct += predicted.eq(labels).sum().item()


    train_acc = 100. * correct / total # 학습 정확도(%)


    # 평가
    model.eval() # 평가 모드
    val_correct = 0
    val_total = 0
    with torch.no_grad(): # 평가 중 gradient 계산 안 함
        for images, labels in testloader:
            images, labels = images.to(device), labels.to(device)
            outputs = model(images)
            _, predicted = outputs.max(1) # 예측 클래스
            val_total += labels.size(0)
            val_correct += predicted.eq(labels).sum().item()


    val_acc = 100. * val_correct / val_total # 검증 정확도(%)
    scheduler.step() # 학습률 갱신


    if val_acc > best_acc: # 최고 기록 갱신 시 저장
        best_acc = val_acc
        torch.save(model.state_dict(), 'outputs/best_resnet18_cifar10.pth') # 모델 가중치 저장


    writer.add_scalar('Loss/train', running_loss/len(trainloader), epoch + 5)
    writer.add_scalar('Accuracy/train', train_acc, epoch + 5)
    writer.add_scalar('Accuracy/val', val_acc, epoch + 5)


    print(f"Epoch {epoch+1}: Train Acc={train_acc:.2f}%, Val Acc={val_acc:.2f}%"
          + (" *" if val_acc == best_acc else ""))


writer.close()
print(f"\n최고 검증 정확도: {best_acc:.2f}%")
print("\n 실습 4 완료!")
```


**실행**:
```bash
python practice_resnet18.py
```


**예상 결과**:
- ResNet-18 Fine-tuning: 약 88-92% 정확도


---


## 실습 5: Checkpoint 로드 및 추론

앞 실습들이 모델을 만들고 저장했다면, 이 실습은 저장된 가중치를 다시 불러와 실제로 쓰는 쪽이다. 학습과 배포/추론은 분리된 과정임을 경험한다.

### 왜 로드를 따로 연습하나

학습은 한 번, 추론은 수없이 한다. 실무 배포는 학습된 `.pth`/`.pt`를 로드해 새 입력에 예측하는 것이 전부다. 이 실습은 그 최소 형태다. 빈 모델 구조 생성 -> 가중치 로드 -> `eval()` -> 추론. 주의점이 하나 있다. 저장은 가중치(state_dict)만 한다. 로드하려면 학습할 때와 똑같은 모델 구조를 먼저 만들어야 한다(특히 `fc` 교체를 동일하게). 구조가 다르면 로드가 실패한다. 이것이 실무에서 자주 막히는 지점이다.

### 학습 목표

| 목표 | 코드에서 해당 부분 | 무엇을 익히나 |
|------|------------------|--------------|
| 구조 복원 | `resnet18()` + `fc` 교체 동일하게 | state_dict는 구조가 같아야 로드된다 |
| 가중치 로드 | `load_state_dict(torch.load(...))` | `map_location`으로 GPU/CPU 환경 호환 |
| 추론 모드 | `model.eval()` + `no_grad` | 학습과 분리, BatchNorm 통계 고정 |
| 결과 해석 | `softmax` -> `argmax`, confidence | 점수를 확률로, 최고 확률이 예측 |

### 핵심 포인트

- 시각화에서 역정규화(`img * std + mean`)를 하는 이유는 학습 입력이 정규화돼 색이 깨져 보이기 때문이다. 사람이 보려면 정규화를 되돌려야 한다.
- 예측이 맞으면 초록, 틀리면 빨강으로 제목 색을 바꿔 모델이 어디서 틀리는지 눈으로 점검한다(sanity check).


**파일명**: `practice_inference.py`


```python
"""
실습 5: 저장된 모델 로드 후 추론
목표: Checkpoint를 로드하고 개별 이미지에 대해 추론한다.
"""
import os # 파일/디렉토리 작업
import torch
import torch.nn as nn # 신경망 레이어
import torchvision # 이미지 데이터셋/모델 모음
import torchvision.transforms as transforms # 이미지 전처리
import torchvision.models as models # 사전학습된 모델 (ResNet 등)
import matplotlib
matplotlib.use('Agg') # GUI 없이 파일로만 저장하는 백엔드 (서버 환경용)
import matplotlib.pyplot as plt


print("=" * 50)
print("실습 5: 모델 로드 및 추론")
print("=" * 50)


device = torch.device('cuda' if torch.cuda.is_available() else 'cpu') # GPU 있으면 GPU 사용
classes = ('airplane', 'automobile', 'bird', 'cat', 'deer', # 클래스 인덱스 -> 이름 변환용
           'dog', 'frog', 'horse', 'ship', 'truck')
os.makedirs('outputs', exist_ok=True) # 결과물 폴더 (추론 결과를 수업 자료와 분리)


# -- 모델 로드 --
model = models.resnet18() # 빈 ResNet-18 구조 생성 (가중치는 랜덤)
model.fc = nn.Linear(model.fc.in_features, 10) # 학습 때와 동일하게 FC 교체 (구조가 같아야 가중치 로드 가능)
model.load_state_dict(torch.load('outputs/best_resnet18_cifar10.pth', map_location=device)) # 저장된 학습 가중치 로드
model = model.to(device)
model.eval() # 평가 모드 (추론 시 필수)
print("모델 로드 완료!")


# -- 테스트 데이터 --
transform = transforms.Compose([ # 학습 때와 동일한 전처리 (Resize + 정규화)
    transforms.Resize(224),
    transforms.ToTensor(),
    transforms.Normalize((0.4914, 0.4822, 0.4465),
                         (0.2470, 0.2435, 0.2616)),
])


testset = torchvision.datasets.CIFAR10( # 테스트셋 (추론 대상)
    root='./data', train=False, download=True, transform=transform)


# -- 랜덤 샘플 추론 --
fig, axes = plt.subplots(2, 5, figsize=(15, 6)) # 2x5 그리드 (이미지 10장 표시용)
indices = torch.randint(0, len(testset), (10,)) # 테스트셋에서 무작위 10개 인덱스 선택


for i, idx in enumerate(indices):
    image, label = testset[idx] # idx번째 (이미지, 정답 라벨) 가져오기


    with torch.no_grad(): # 추론이므로 gradient 계산 끔
        output = model(image.unsqueeze(0).to(device)) # 배치 차원 추가 후 추론 -> 클래스 점수
        prob = torch.softmax(output, dim=1) # 점수를 확률로 변환 (합 1)
        pred_class = prob.argmax(1).item() # 확률이 가장 높은 클래스 = 예측
        confidence = prob.max().item() # 그 예측의 확신도 (0-1)


    ax = axes[i // 5, i % 5] # i번째 이미지를 놓을 칸 위치 계산
    # 시각화를 위해 역정규화
    img_show = image.permute(1, 2, 0).numpy() # CHW -> HWC (matplotlib 표시용)
    img_show = img_show * [0.2470, 0.2435, 0.2616] + [0.4914, 0.4822, 0.4465] # 정규화 역연산 (원래 색 복원)
    img_show = img_show.clip(0, 1) # 0-1 범위 밖 값 잘라냄
    ax.imshow(img_show)


    color = 'green' if pred_class == label else 'red' # 맞으면 초록, 틀리면 빨강
    ax.set_title(f"Pred: {classes[pred_class]}\n"
                 f"True: {classes[label]}\n"
                 f"Conf: {confidence:.2f}",
                 color=color, fontsize=9)
    ax.axis('off')


plt.tight_layout()
plt.savefig('outputs/inference_results.png', dpi=100) # 결과를 이미지 파일로 저장
print("추론 결과 저장: outputs/inference_results.png")
print("\n 실습 5 완료!")
```


**실행**:
```bash
python practice_inference.py
```


---


## 실습 체크리스트


- [x] CUDA 환경 세팅 완료 (`torch.cuda.is_available()` = True)
- [x] Tensor 연산 및 shape 변환 숙지
- [x] autograd로 gradient 계산 원리 이해
- [x] CIFAR-10 SimpleCNN 학습 (75%+ 달성)
- [x] ResNet-18 Fine-tuning (85%+ 달성)
- [x] TensorBoard로 학습 커브 확인
- [x] Checkpoint 저장/로드 성공
- [x] 추론 결과 시각화


---


## 참고 자료


- [PyTorch Tutorials](https://pytorch.org/tutorials/)
- [PyTorch 공식 문서](https://pytorch.org/docs/stable/index.html)
- [TensorBoard with PyTorch](https://pytorch.org/docs/stable/tensorboard.html)
- [CIFAR-10 Dataset](https://www.cs.toronto.edu/~kriz/cifar.html)
