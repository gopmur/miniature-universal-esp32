import errno
import os
import sys
import shutil
import gzip
import mimetypes

def _compress_assets(input_path: str, rel_path: str, output_path: str, asset_uris: dict[str, str]):
  full_path = f"{input_path}/{rel_path}"
  entries = os.scandir(full_path)
  for entry in entries:
    if entry.is_dir():
      new_rel_path = f"{rel_path}/{entry.name}" if rel_path else entry.name
      _compress_assets(
          input_path, new_rel_path, output_path, asset_uris)
    elif entry.is_file():
      file_rel_path = f"{rel_path}/{entry.name}" if rel_path else entry.name
      uri = f"/{rel_path}" if entry.name == "index.html" else f"/{file_rel_path}"
      compressed_file_name = f"{file_rel_path}.gz".replace(
          "/", "_").replace("-", "_")
      asset_uris[compressed_file_name] = uri
      with open(entry.path, "rb") as file:
        with gzip.open(f"{output_path}/assets/{compressed_file_name}", "wb") as compress_file:
          shutil.copyfileobj(file, compress_file)


def compress_assets(input_path: str, output_path: str, asset_uris: dict[str, str]):
  _compress_assets(input_path, "", output_path, asset_uris)


def generate_c_code(output_path: str, asset_uris: dict[str, str]):
  output = ""
  output += f"""#pragma once
#include "esp_http_server.h"
#include "esp_err.h"
#define DEFINE_FILE_GET_HANDLER(file_name, type) \\
  static esp_err_t file_name##_get(httpd_req_t* req) {{ \\
    extern const char file_name##_start[] asm("_binary_" #file_name "_start"); \\
    extern const char file_name##_file_end[] asm("_binary_" #file_name \\
                                                 "_end"); \\
    const size_t size = file_name##_file_end - file_name##_start; \\
    esp_err_t res = httpd_resp_set_type(req, type); \\
    if (res) {{ \\
      return res; \\
    }} \\
    res = httpd_resp_set_hdr(req, "Content-Encoding", "gzip"); \\
    if (res) {{ \\
      return res; \\
    }} \\
    return httpd_resp_send(req, file_name##_start, size); \\
  }}

#define REGISTER_FILE_URI(server_instance, file_name, file_uri) \\
  {{ \\
    static const httpd_uri_t file_name##_uri = {{ \\
        .uri = file_uri, \\
        .method = HTTP_GET, \\
        .handler = file_name##_get, \\
        .user_ctx = NULL, \\
    }}; \\
    httpd_register_uri_handler(server_instance, &file_name##_uri); \\
  }}
"""
  
  entries = os.scandir(f"{output_path}/assets")
  for entry in entries:
    if not entry.is_file or not entry.name.endswith(".gz"):
      continue
    c_name = entry.name.replace(".", "_").replace("-", "_")
    output += f"DEFINE_FILE_GET_HANDLER({c_name}, \"{mimetypes.guess_type(entry.name[:-3])[0]}\")\n"
  
  output += "inline void http_server_register_assets(httpd_handle_t http_server) {\n"

  entries = os.scandir(f"{output_path}/assets")
  for entry in entries:
    uri = asset_uris[entry.name]
    c_name = entry.name.replace(".", "_").replace("-", "_")
    output += f"  REGISTER_FILE_URI(http_server, {c_name}, \"{uri}\");\n"
  output += "}"

  return output


if __name__ == "__main__":
  if len(sys.argv) != 3:
    print("In correct number of arguments")
    exit(errno.EINVAL)
  input_path = sys.argv[1]
  output_path = sys.argv[2]
  if not os.path.exists(input_path):
    print("Provided input path was invalid")
    exit(errno.EINVAL)
  if not os.path.exists(f"{output_path}/assets"):
    os.makedirs(f"{output_path}/assets")
  if not os.path.exists(f"{output_path}/inc"):
    os.makedirs(f"{output_path}/inc")
  asset_uris: dict[str, str] = {}
  compress_assets(input_path, output_path, asset_uris)
  generated_code = generate_c_code(output_path, asset_uris)
  with open(f"{output_path}/inc/http_assets.hpp", "w") as http_h:
    http_h.write(generated_code)