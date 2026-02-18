import numpy as np
from PIL import Image


filename = "../../../output/Result_0/model_layout_transform_0_0.raw"
raw = np.fromfile(filename, dtype=np.float32)
logits = raw.reshape(1, 750, 1000, 21)[0]
logits = np.transpose(logits, (2, 0, 1))

print("logits shape: ", logits.shape)
print(logits[0, 0, :10])


palette = np.array([2 ** 25 - 1, 2 ** 15 - 1, 2 ** 21 - 1], dtype=np.uint32)
colors = (np.arange(21, dtype=np.uint32)[:, None] * palette) % 255
colors = colors.astype(np.uint8).reshape(-1).tolist()
colors.extend([0] * (256 * 3 - len(colors)))

predictions = np.argmax(logits, axis=0).astype(np.uint8)
r = Image.fromarray(predictions).resize((1000, 750))
r.putpalette(colors)
r.save("output_qnn.png")
