from pathlib import Path
import argparse

import numpy as np
from PIL import Image


def png_to_raw(input_path: Path, output_path: Path) -> None:
    img = Image.open(input_path).convert("RGB")
    arr = np.asarray(img, dtype=np.uint8)  # [H, W, C]
    print(arr.shape)
    chw = np.transpose(arr, (2, 0, 1))     # [C, H, W]: [3, 979, 1490]
    chw = chw.astype(np.float32, copy=False)
    np.multiply(chw, 1.0 / 255.0, out=chw)
    chw.tofile(output_path)


def main() -> None:
    parser = argparse.ArgumentParser(description="Convert PNG image to CHW .raw")
    parser.add_argument("--input", type=Path, default=Path("test.png"), help="Input PNG path")
    parser.add_argument("--output", type=Path, default=Path("test.raw"), help="Output RAW path")
    args = parser.parse_args()

    png_to_raw(args.input, args.output)
    print(f"Saved {args.output} with shape [C, H, W]")


if __name__ == "__main__":
    main()
