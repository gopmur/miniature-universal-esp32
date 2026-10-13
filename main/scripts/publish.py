import os
import shutil
import subprocess
import json
import glob
import paramiko
from scp import SCPClient


class ProjectData:
    def __init__(self, project_version: str, project_name: str):
        self.version = project_version
        self.name = project_name


def get_project_data():
    query_dir = os.path.join("build", ".cmake", "api", "v1", "query")
    os.makedirs(query_dir, exist_ok=True)

    with open(os.path.join(query_dir, "codemodel-v2"), "w") as f:
        pass  # The presence of this file triggers the JSON output

    # 2. Run CMake to configure the project (this generates the JSON)
    # subprocess.run(["idf.py", "clean", "build"])

    # 3. Locate and read the generated JSON reply
    reply_dir = os.path.join("build", ".cmake", "api", "v1", "reply")
    codemodel_files = glob.glob(
        os.path.join(reply_dir, "target-__idf_esp_app_format-*.json")
    )

    if not codemodel_files:
        raise FileNotFoundError("CMake did not generate the codemodel JSON.")

    with open(codemodel_files[0], "r") as f:
        data = json.load(f)

    defines: list = data["compileGroups"][0]["defines"]
    project_ver_define = list(
        filter(lambda d: d["define"].strip().startswith("PROJECT_VER="), defines)
    )
    project_name_define = list(
        filter(lambda d: d["define"].strip().startswith("PROJECT_NAME="), defines)
    )
    if len(project_ver_define) == 0:
        print("could not find project version")
        exit(-1)
    if len(project_name_define) == 0:
        print("could not find project name")
        exit(-1)

    project_ver_define_str: str = project_ver_define[0]["define"]
    ver_start_index = project_ver_define_str.index('"')
    ver_end_index = project_ver_define_str.index('"', ver_start_index + 1)
    version = project_ver_define_str[ver_start_index + 1 : ver_end_index]

    project_name_define_str: str = project_name_define[0]["define"]
    name_start_index = project_name_define_str.index('"')
    name_end_index = project_name_define_str.index('"', name_start_index + 1)
    name = project_name_define_str[name_start_index + 1 : name_end_index]
    project_data = ProjectData(version, name)
    return project_data


def upload_firmware(firmware_path: str):
    server = "185.239.2.51"
    user = "ubuntu"

    ssh = paramiko.SSHClient()
    ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())


    print(f"Connecting to {server}...")
    ssh.connect(server, username=user, password="YOUR_PASSWORD_HERE")

    local_file = firmware_path
    remote_path = "/home/ubuntu/fumcare-rp-update-server/firmware/"

    print(f"Uploading {local_file} to {remote_path}...")
    with SCPClient(ssh.get_transport()) as scp:
        scp.put(local_file, remote_path)

    print("Upload complete!")

    ssh.close()


def main():
    project_data = get_project_data()
    dist_name = f"{project_data.version}.bin"
    try:
        os.mkdir("dist")
    except:
        pass
    firmware_path = f"dist/{dist_name}"
    shutil.copy(f"build/{project_data.name}.bin", f"dist/{dist_name}")
    upload_firmware(firmware_path)

if __name__ == "__main__":
    main()
