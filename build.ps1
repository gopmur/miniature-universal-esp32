# Set environment variable
# $env:IDF_TOOLCHAIN = "clang"

# Remove the build/generated directory if it exists
if (Test-Path .\build\generated) {
    Remove-Item -Recurse -Force .\build\generated
}

# Run the Python script with arguments
python .\main\scripts\generate_http_assets.py .\main\assets .\build\generated

# Check if IDF_PATH environment variable is set; if not, source export.ps1
if (-not $env:IDF_PATH) {
    # Adjust the path to your esp-idf export.ps1 script below:
    . "$HOME\esp\v5.4.2\esp-idf\export.ps1"
}

# Build the project
idf.py build
