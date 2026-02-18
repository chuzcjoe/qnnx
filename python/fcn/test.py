import torch
import urllib
import numpy as np
from PIL import Image
import torch.nn as nn

class FCNOutOnly(nn.Module):
    def __init__(self, model):
        super().__init__()
        self.model = model

    def forward(self, x):
        return self.model(x)["out"]  # [N, C, H, W]

base_model = torch.hub.load('pytorch/vision:v0.10.0', 'fcn_resnet50', pretrained=True)
model = FCNOutOnly(base_model).eval()

filename = "park.jpg"
input_image = Image.open(filename)
input_image = input_image.convert("RGB")
print("input image: ", input_image.size)
input_array = np.asarray(input_image, dtype=np.float32) / 255.0
mean = np.array([0.485, 0.456, 0.406], dtype=np.float32)
std = np.array([0.229, 0.224, 0.225], dtype=np.float32)
input_array = (input_array - mean) / std
input_array = np.transpose(input_array, (2, 0, 1))
with open("./test.raw", "wb") as f:
    f.write(np.ascontiguousarray(np.transpose(input_array, (1, 2, 0))))
input_tensor = torch.from_numpy(np.ascontiguousarray(input_array))
input_batch = input_tensor.unsqueeze(0) # create a mini-batch as expected by the model
print("input batch size: ", input_batch.shape)

# move the input and model to GPU for speed if available
if torch.cuda.is_available():
    input_batch = input_batch.to('cuda')
    model.to('cuda')

with torch.no_grad():
    output = model(input_batch)[0]
    print("output size: ", output.shape)
    print(output[0, 0, :10])
output_predictions = output.argmax(0)

palette = np.array([2 ** 25 - 1, 2 ** 15 - 1, 2 ** 21 - 1], dtype=np.uint32)
colors = (np.arange(21, dtype=np.uint32)[:, None] * palette) % 255
colors = colors.astype(np.uint8).reshape(-1).tolist()
colors.extend([0] * (256 * 3 - len(colors)))

# plot the semantic segmentation predictions of 21 classes in each color
r = Image.fromarray(output_predictions.byte().cpu().numpy()).resize(input_image.size)
r.putpalette(colors)
r.save("output_python.png")
