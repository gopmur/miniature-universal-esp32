import errno
import os
import sys
import shutil
import gzip

def _compress_assets(input_path: str, rel_path: str, output_path: str):
  full_path = f"{input_path}/{rel_path}"
  entries = os.scandir(full_path)
  for entry in entries:
    if entry.is_dir():
      new_rel_path = f"{rel_path}/{entry.name}" if rel_path else entry.name
      _compress_assets(
          input_path, new_rel_path, output_path)
    elif entry.is_file():
      file_rel_path = f"{rel_path}/{entry.name}" if rel_path else entry.name 
      compressed_file_name = f"{file_rel_path}.gz".replace("/", "_").replace("-", "_")
      with open(entry.path, "rb") as file:
        with gzip.open(f"{output_path}/{compressed_file_name}", "wb") as compress_file:
          shutil.copyfileobj(file, compress_file)


def compress_assets(input_path: str, output_path: str):
  _compress_assets(input_path, "", output_path)


if __name__ == "__main__":
  if len(sys.argv) != 3:
    print("In correct number of arguments")
    exit(errno.EINVAL)
  input_path = sys.argv[1]
  output_path = sys.argv[2]
  try:
    os.listdir(input_path)
  except:
    print("Provided input path was invalid")
    exit(errno.EINVAL)
  if not os.path.exists(output_path):
    os.makedirs(output_path)
  compress_assets(input_path, output_path)
