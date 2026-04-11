# Week 1 실습: PyTorch 기초 재정비

> [goal] **실습 목표**: CUDA 환경 세팅 → Tensor 연산 → CNN 학습 → ResNet Fine-tuning → TensorBoard → Checkpoint
> [time] **예상 시간**: 6~8시간

---

## [tool] 환경 설정

```bash
# 가상환경 생성 및 활성화
conda create -n phase5 python=3.10 -y
conda activate phase5

# 패키지 설치
pip install -r requirements.txt

# GPU 확인
python -c "import torch; print(f'CUDA: {torch.cuda.is_available()}, Device: {torch.cuda.get_device_name(0) if torch.cuda.is_available() else \"CPU\"}')"
```

---

## [note] 실습 1: Tensor 기초 연산

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
zeros = torch.zeros(3, 4)
ones = torch.ones(2, 3)
rand = torch.randn(3, 3)  # 표준정규분포
arange = torch.arange(0, 10, 2)
linspace = torch.linspace(0, 1, 5)

print(f"  zeros shape: {zeros.shape}")
print(f"  randn:\n{rand}")
print(f"  arange: {arange}")
print(f"  linspace: {linspace}")

# -- 1-2. NumPy 변환 --
print("\n[1-2] NumPy 변환")
np_array = np.array([[1, 2], [3, 4]], dtype=np.float32)
tensor_from_np = torch.from_numpy(np_array)
np_from_tensor = tensor_from_np.numpy()
print(f"  NumPy → Tensor: {tensor_from_np}")
print(f"  Tensor → NumPy: {np_from_tensor}")

# -- 1-3. 연산 --
print("\n[1-3] 연산")
a = torch.tensor([[1.0, 2.0], [3.0, 4.0]])
b = torch.tensor([[5.0, 6.0], [7.0, 8.0]])

print(f"  a + b = {a + b}")
print(f"  a * b (요소별) = {a * b}")
print(f"  a @ b (행렬곱) = \n{a @ b}")
print(f"  a.sum() = {a.sum()}")
print(f"  a.mean() = {a.mean()}")
print(f"  a.max() = {a.max()}")

# -- 1-4. Shape 변환 --
print("\n[1-4] Shape 변환")
x = torch.randn(2, 3, 4)
print(f"  원래 shape: {x.shape}")
print(f"  view(2, 12): {x.view(2, 12).shape}")
print(f"  reshape(6, 4): {x.reshape(6, 4).shape}")
print(f"  permute(2,0,1): {x.permute(2, 0, 1).shape}")
print(f"  unsqueeze(0): {x.unsqueeze(0).shape}")
print(f"  squeeze 후: {x.unsqueeze(0).squeeze(0).shape}")

# -- 1-5. GPU 이동 --
print("\n[1-5] GPU 이동")
device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
print(f"  사용 디바이스: {device}")
x_gpu = x.to(device)
print(f"  텐서 디바이스: {x_gpu.device}")

# -- 1-6. 이미지 텐서 변환 연습 --
print("\n[1-6] 이미지 텐서 변환")
# OpenCV 형식: (H, W, C), uint8, BGR
fake_img = np.random.randint(0, 255, (480, 640, 3), dtype=np.uint8)
# PyTorch 형식: (C, H, W), float32, [0, 1]
img_tensor = torch.from_numpy(fake_img).permute(2, 0, 1).float() / 255.0
print(f"  OpenCV shape: {fake_img.shape} → PyTorch shape: {img_tensor.shape}")
print(f"  dtype: {img_tensor.dtype}, range: [{img_tensor.min():.2f}, {img_tensor.max():.2f}]")

print("\n[O] 실습 1 완료!")
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

[O] 실습 1 완료!
```

---

## [note] 실습 2: autograd 자동 미분

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
x = torch.tensor(3.0, requires_grad=True)
y = x ** 2 + 2 * x + 1  # y = x^2 + 2x + 1
y.backward()
print(f"  x = {x.item()}")
print(f"  y = x^2 + 2x + 1 = {y.item()}")
print(f"  dy/dx = 2x + 2 = {x.grad.item()}")  # 2*3 + 2 = 8

# -- 2-2. 벡터 입력 --
print("\n[2-2] 벡터 입력")
x = torch.tensor([1.0, 2.0, 3.0], requires_grad=True)
y = (x ** 2).sum()  # y = x1^2 + x2^2 + x3^2
y.backward()
print(f"  x = {x.data}")
print(f"  y = sum(x^2) = {y.item()}")
print(f"  dy/dx = 2x = {x.grad}")  # [2, 4, 6]

# -- 2-3. 간단한 선형 회귀 --
print("\n[2-3] 수동 선형 회귀 (y = 2x + 1)")
torch.manual_seed(42)

# 데이터 생성: y = 2x + 1 + noise
X = torch.randn(100, 1)
y_true = 2 * X + 1 + 0.1 * torch.randn(100, 1)

# 학습할 파라미터
w = torch.randn(1, requires_grad=True)
b = torch.zeros(1, requires_grad=True)
lr = 0.1

for epoch in range(100):
    # 순전파
    y_pred = X * w + b
    loss = ((y_pred - y_true) ** 2).mean()  # MSE

    # 역전파
    loss.backward()

    # 파라미터 업데이트 (gradient descent)
    with torch.no_grad():
        w -= lr * w.grad
        b -= lr * b.grad

    # gradient 초기화
    w.grad.zero_()
    b.grad.zero_()

    if (epoch + 1) % 20 == 0:
        print(f"  Epoch {epoch+1:3d}: loss={loss.item():.4f}, w={w.item():.4f}, b={b.item():.4f}")

print(f"\n  최종: w={w.item():.4f} (정답: 2.0), b={b.item():.4f} (정답: 1.0)")

# -- 2-4. no_grad 블록 --
print("\n[2-4] torch.no_grad() 사용")
x = torch.randn(3, requires_grad=True)
print(f"  requires_grad 상태: {x.requires_grad}")

with torch.no_grad():
    y = x * 2
    print(f"  no_grad 블록 내 y.requires_grad: {y.requires_grad}")

print("\n[O] 실습 2 완료!")
```

**실행**:
```bash
python practice_autograd.py
```

---

## [note] 실습 3: CIFAR-10 CNN 학습

**파일명**: `practice_cifar10.py`

```python
"""
실습 3: CIFAR-10 CNN 학습
목표: Dataset, DataLoader, 학습 루프, 평가를 직접 구현한다.
"""
import torch
import torch.nn as nn
import torch.optim as optim
import torchvision
import torchvision.transforms as transforms
from torch.utils.data import DataLoader
from torch.utils.tensorboard import SummaryWriter
import time

print("=" * 50)
print("실습 3: CIFAR-10 CNN 학습")
print("=" * 50)

# -- 설정 --
device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
print(f"디바이스: {device}")

# -- 데이터 준비 --
transform_train = transforms.Compose([
    transforms.RandomHorizontalFlip(),
    transforms.RandomCrop(32, padding=4),
    transforms.ToTensor(),
    transforms.Normalize((0.4914, 0.4822, 0.4465),
                         (0.2470, 0.2435, 0.2616)),
])

transform_test = transforms.Compose([
    transforms.ToTensor(),
    transforms.Normalize((0.4914, 0.4822, 0.4465),
                         (0.2470, 0.2435, 0.2616)),
])

trainset = torchvision.datasets.CIFAR10(
    root='./data', train=True, download=True, transform=transform_train)
testset = torchvision.datasets.CIFAR10(
    root='./data', train=False, download=True, transform=transform_test)

trainloader = DataLoader(trainset, batch_size=128, shuffle=True, num_workers=2)
testloader = DataLoader(testset, batch_size=128, shuffle=False, num_workers=2)

classes = ('airplane', 'automobile', 'bird', 'cat', 'deer',
           'dog', 'frog', 'horse', 'ship', 'truck')

print(f"학습 데이터: {len(trainset)}장, 테스트 데이터: {len(testset)}장")

# -- CNN 모델 --
class SimpleCNN(nn.Module):
    def __init__(self):
        super().__init__()
        self.features = nn.Sequential(
            nn.Conv2d(3, 32, 3, padding=1),
            nn.BatchNorm2d(32),
            nn.ReLU(),
            nn.MaxPool2d(2),

            nn.Conv2d(32, 64, 3, padding=1),
            nn.BatchNorm2d(64),
            nn.ReLU(),
            nn.MaxPool2d(2),

            nn.Conv2d(64, 128, 3, padding=1),
            nn.BatchNorm2d(128),
            nn.ReLU(),
            nn.AdaptiveAvgPool2d(1),
        )
        self.classifier = nn.Linear(128, 10)

    def forward(self, x):
        x = self.features(x)
        x = x.view(x.size(0), -1)
        x = self.classifier(x)
        return x

model = SimpleCNN().to(device)
criterion = nn.CrossEntropyLoss()
optimizer = optim.Adam(model.parameters(), lr=0.001)
scheduler = optim.lr_scheduler.StepLR(optimizer, step_size=15, gamma=0.5)

# -- TensorBoard --
writer = SummaryWriter('runs/cifar10_simple_cnn')

# -- 학습 함수 --
def train_one_epoch(model, loader, criterion, optimizer, device):
    model.train()
    running_loss = 0.0
    correct = 0
    total = 0

    for images, labels in loader:
        images, labels = images.to(device), labels.to(device)

        optimizer.zero_grad()
        outputs = model(images)
        loss = criterion(outputs, labels)
        loss.backward()
        optimizer.step()

        running_loss += loss.item()
        _, predicted = outputs.max(1)
        total += labels.size(0)
        correct += predicted.eq(labels).sum().item()

    return running_loss / len(loader), 100. * correct / total

# -- 평가 함수 --
@torch.no_grad()
def evaluate(model, loader, criterion, device):
    model.eval()
    running_loss = 0.0
    correct = 0
    total = 0

    for images, labels in loader:
        images, labels = images.to(device), labels.to(device)
        outputs = model(images)
        loss = criterion(outputs, labels)

        running_loss += loss.item()
        _, predicted = outputs.max(1)
        total += labels.size(0)
        correct += predicted.eq(labels).sum().item()

    return running_loss / len(loader), 100. * correct / total

# -- 학습 루프 --
num_epochs = 30
best_acc = 0.0

print(f"\n{'Epoch':>6} | {'Train Loss':>10} | {'Train Acc':>9} | {'Val Loss':>8} | {'Val Acc':>7} | {'Time':>6}")
print("-" * 65)

for epoch in range(num_epochs):
    start = time.time()

    train_loss, train_acc = train_one_epoch(
        model, trainloader, criterion, optimizer, device)
    val_loss, val_acc = evaluate(
        model, testloader, criterion, device)

    scheduler.step()
    elapsed = time.time() - start

    # TensorBoard 로깅
    writer.add_scalar('Loss/train', train_loss, epoch)
    writer.add_scalar('Loss/val', val_loss, epoch)
    writer.add_scalar('Accuracy/train', train_acc, epoch)
    writer.add_scalar('Accuracy/val', val_acc, epoch)
    writer.add_scalar('LR', optimizer.param_groups[0]['lr'], epoch)

    # Checkpoint 저장
    if val_acc > best_acc:
        best_acc = val_acc
        torch.save({
            'epoch': epoch,
            'model_state_dict': model.state_dict(),
            'optimizer_state_dict': optimizer.state_dict(),
            'best_acc': best_acc,
        }, 'best_cifar10_cnn.pth')
        marker = ' *'
    else:
        marker = ''

    print(f"{epoch+1:>6} | {train_loss:>10.4f} | {train_acc:>8.2f}% | "
          f"{val_loss:>8.4f} | {val_acc:>6.2f}% | {elapsed:>5.1f}s{marker}")

writer.close()
print(f"\n최고 검증 정확도: {best_acc:.2f}%")
print("TensorBoard 확인: tensorboard --logdir=runs")
print("\n[O] 실습 3 완료!")
```

**실행**:
```bash
python practice_cifar10.py

# TensorBoard 확인
tensorboard --logdir=runs
```

**예상 결과**:
- SimpleCNN: 약 75~80% 정확도

---

## [note] 실습 4: ResNet-18 Fine-tuning

**파일명**: `practice_resnet18.py`

```python
"""
실습 4: ResNet-18 Fine-tuning
목표: Pretrained 모델을 CIFAR-10에 맞게 수정하고 학습한다.
"""
import torch
import torch.nn as nn
import torch.optim as optim
import torchvision
import torchvision.transforms as transforms
from torch.utils.data import DataLoader
from torch.utils.tensorboard import SummaryWriter
import torchvision.models as models
import time

print("=" * 50)
print("실습 4: ResNet-18 Fine-tuning")
print("=" * 50)

device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')

# -- 데이터 (ResNet용 224x224) --
transform_train = transforms.Compose([
    transforms.Resize(224),
    transforms.RandomHorizontalFlip(),
    transforms.ToTensor(),
    transforms.Normalize((0.4914, 0.4822, 0.4465),
                         (0.2470, 0.2435, 0.2616)),
])

transform_test = transforms.Compose([
    transforms.Resize(224),
    transforms.ToTensor(),
    transforms.Normalize((0.4914, 0.4822, 0.4465),
                         (0.2470, 0.2435, 0.2616)),
])

trainset = torchvision.datasets.CIFAR10(
    root='./data', train=True, download=True, transform=transform_train)
testset = torchvision.datasets.CIFAR10(
    root='./data', train=False, download=True, transform=transform_test)

trainloader = DataLoader(trainset, batch_size=64, shuffle=True, num_workers=2)
testloader = DataLoader(testset, batch_size=64, shuffle=False, num_workers=2)

# -- ResNet-18 모델 (Pretrained) --
model = models.resnet18(weights='IMAGENET1K_V1')

# 마지막 FC 레이어를 CIFAR-10용으로 교체
num_features = model.fc.in_features
model.fc = nn.Linear(num_features, 10)

model = model.to(device)

# -- Backbone freeze (선택사항) --
# 처음 몇 에폭은 FC만 학습 → 이후 전체 학습
for param in model.parameters():
    param.requires_grad = False
for param in model.fc.parameters():
    param.requires_grad = True

criterion = nn.CrossEntropyLoss()
optimizer = optim.Adam(model.fc.parameters(), lr=0.001)

writer = SummaryWriter('runs/cifar10_resnet18')

# -- Phase 1: FC만 학습 (5 epochs) --
print("\n-- Phase 1: FC 레이어만 학습 (Backbone Frozen) --")
for epoch in range(5):
    model.train()
    running_loss = 0.0
    correct = 0
    total = 0

    for images, labels in trainloader:
        images, labels = images.to(device), labels.to(device)
        optimizer.zero_grad()
        outputs = model(images)
        loss = criterion(outputs, labels)
        loss.backward()
        optimizer.step()

        running_loss += loss.item()
        _, predicted = outputs.max(1)
        total += labels.size(0)
        correct += predicted.eq(labels).sum().item()

    acc = 100. * correct / total
    print(f"  Epoch {epoch+1}: Loss={running_loss/len(trainloader):.4f}, Acc={acc:.2f}%")

# -- Phase 2: 전체 모델 학습 (Unfreeze) --
print("\n-- Phase 2: 전체 모델 학습 (Backbone Unfrozen) --")
for param in model.parameters():
    param.requires_grad = True

optimizer = optim.Adam(model.parameters(), lr=0.0001)  # 작은 lr
scheduler = optim.lr_scheduler.CosineAnnealingLR(optimizer, T_max=15)

best_acc = 0.0
for epoch in range(15):
    model.train()
    running_loss = 0.0
    correct = 0
    total = 0

    for images, labels in trainloader:
        images, labels = images.to(device), labels.to(device)
        optimizer.zero_grad()
        outputs = model(images)
        loss = criterion(outputs, labels)
        loss.backward()
        optimizer.step()

        running_loss += loss.item()
        _, predicted = outputs.max(1)
        total += labels.size(0)
        correct += predicted.eq(labels).sum().item()

    train_acc = 100. * correct / total

    # 평가
    model.eval()
    val_correct = 0
    val_total = 0
    with torch.no_grad():
        for images, labels in testloader:
            images, labels = images.to(device), labels.to(device)
            outputs = model(images)
            _, predicted = outputs.max(1)
            val_total += labels.size(0)
            val_correct += predicted.eq(labels).sum().item()

    val_acc = 100. * val_correct / val_total
    scheduler.step()

    if val_acc > best_acc:
        best_acc = val_acc
        torch.save(model.state_dict(), 'best_resnet18_cifar10.pth')

    writer.add_scalar('Loss/train', running_loss/len(trainloader), epoch + 5)
    writer.add_scalar('Accuracy/train', train_acc, epoch + 5)
    writer.add_scalar('Accuracy/val', val_acc, epoch + 5)

    print(f"  Epoch {epoch+1}: Train Acc={train_acc:.2f}%, Val Acc={val_acc:.2f}%"
          + (" *" if val_acc == best_acc else ""))

writer.close()
print(f"\n최고 검증 정확도: {best_acc:.2f}%")
print("\n[O] 실습 4 완료!")
```

**실행**:
```bash
python practice_resnet18.py
```

**예상 결과**:
- ResNet-18 Fine-tuning: 약 88~92% 정확도

---

## [note] 실습 5: Checkpoint 로드 및 추론

**파일명**: `practice_inference.py`

```python
"""
실습 5: 저장된 모델 로드 후 추론
목표: Checkpoint를 로드하고 개별 이미지에 대해 추론한다.
"""
import torch
import torch.nn as nn
import torchvision
import torchvision.transforms as transforms
import torchvision.models as models
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

print("=" * 50)
print("실습 5: 모델 로드 및 추론")
print("=" * 50)

device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
classes = ('airplane', 'automobile', 'bird', 'cat', 'deer',
           'dog', 'frog', 'horse', 'ship', 'truck')

# -- 모델 로드 --
model = models.resnet18()
model.fc = nn.Linear(model.fc.in_features, 10)
model.load_state_dict(torch.load('best_resnet18_cifar10.pth', map_location=device))
model = model.to(device)
model.eval()
print("모델 로드 완료!")

# -- 테스트 데이터 --
transform = transforms.Compose([
    transforms.Resize(224),
    transforms.ToTensor(),
    transforms.Normalize((0.4914, 0.4822, 0.4465),
                         (0.2470, 0.2435, 0.2616)),
])

testset = torchvision.datasets.CIFAR10(
    root='./data', train=False, download=True, transform=transform)

# -- 랜덤 샘플 추론 --
fig, axes = plt.subplots(2, 5, figsize=(15, 6))
indices = torch.randint(0, len(testset), (10,))

for i, idx in enumerate(indices):
    image, label = testset[idx]

    with torch.no_grad():
        output = model(image.unsqueeze(0).to(device))
        prob = torch.softmax(output, dim=1)
        pred_class = prob.argmax(1).item()
        confidence = prob.max().item()

    ax = axes[i // 5, i % 5]
    # 시각화를 위해 역정규화
    img_show = image.permute(1, 2, 0).numpy()
    img_show = img_show * [0.2470, 0.2435, 0.2616] + [0.4914, 0.4822, 0.4465]
    img_show = img_show.clip(0, 1)
    ax.imshow(img_show)

    color = 'green' if pred_class == label else 'red'
    ax.set_title(f"Pred: {classes[pred_class]}\n"
                 f"True: {classes[label]}\n"
                 f"Conf: {confidence:.2f}",
                 color=color, fontsize=9)
    ax.axis('off')

plt.tight_layout()
plt.savefig('inference_results.png', dpi=100)
print("추론 결과 저장: inference_results.png")
print("\n[O] 실습 5 완료!")
```

**실행**:
```bash
python practice_inference.py
```

---

## [O] 실습 체크리스트

- [ ] CUDA 환경 세팅 완료 (`torch.cuda.is_available()` = True)
- [ ] Tensor 연산 및 shape 변환 숙지
- [ ] autograd로 gradient 계산 원리 이해
- [ ] CIFAR-10 SimpleCNN 학습 (75%+ 달성)
- [ ] ResNet-18 Fine-tuning (85%+ 달성)
- [ ] TensorBoard로 학습 커브 확인
- [ ] Checkpoint 저장/로드 성공
- [ ] 추론 결과 시각화

---

## [link] 참고 자료

- [PyTorch Tutorials](https://pytorch.org/tutorials/)
- [PyTorch 공식 문서](https://pytorch.org/docs/stable/index.html)
- [TensorBoard with PyTorch](https://pytorch.org/docs/stable/tensorboard.html)
- [CIFAR-10 Dataset](https://www.cs.toronto.edu/~kriz/cifar.html)
