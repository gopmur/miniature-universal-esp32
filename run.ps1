#!/usr/bin/env pwsh

if (.\build.ps1) {
    if (idf.py flash) {
        idf.py monitor
    }
}
