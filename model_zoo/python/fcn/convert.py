import torch
import numpy as np
import torch.nn as nn

class FCNOutOnly(nn.Module):
    def __init__(self, model):
        super().__init__()
        self.model = model

    def forward(self, x):
        return self.model(x)["out"]  # [N, C, H, W]

base_model = torch.hub.load('pytorch/vision:v0.10.0', 'fcn_resnet50', pretrained=True)
model = FCNOutOnly(base_model).eval()
input_image = torch.rand(1, 3, 750, 1000)
scripted_model = torch.jit.trace(model, input_image)
scripted_model.save("fcn.pt")
