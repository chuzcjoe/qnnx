from model import UNet
import torch

if __name__ == '__main__':
    model = UNet(n_channels=3, n_classes=2, bilinear=False)
    device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
    model.to(device=device)

    state_dict = torch.load("./weights.pth", map_location=device)
    mask_values = state_dict.pop('mask_values', [0, 1])
    model.load_state_dict(state_dict)
    model.eval()

    input = torch.rand(1, 3, 979, 1490)
    scripted_model = torch.jit.trace(model, input)
    scripted_model.save("segnet.pt")