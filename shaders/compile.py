import subprocess
from os import chdir, path

if __name__ == "__main__":
    chdir(path.dirname(path.realpath(__file__)))
    subprocess.run("C:/VulkanSDK/1.2.182.0/Bin/glslc.exe shader.vert -o vert.spv".split())
    subprocess.run("C:/VulkanSDK/1.2.182.0/Bin/glslc.exe shader.frag -o frag.spv".split())
