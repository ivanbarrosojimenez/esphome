param(
    [string]$VenvPath = ".venv"
)

Write-Host "Creating venv at $VenvPath and installing dev requirements..."
python -m venv $VenvPath
$activate = "$VenvPath\Scripts\Activate.ps1"
Write-Host "Activating venv: $activate"
. $activate
pip install --upgrade pip
pip install -r requirements-dev.txt
Write-Host "Test environment ready. Activate with: . $activate and run: pytest -q"