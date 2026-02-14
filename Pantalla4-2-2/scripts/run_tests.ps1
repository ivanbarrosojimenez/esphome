param(
    [string]$VenvPath = ".venv",
    [switch]$Coverage
)

$activate = "$VenvPath\Scripts\Activate.ps1"
if (Test-Path $activate) {
  . $activate
} else {
  Write-Host "Virtualenv not found. Run scripts\setup_test_env.ps1 first." -ForegroundColor Yellow
  exit 1
}

if ($Coverage) {
  coverage run -m pytest -q
  coverage report -m
} else {
  pytest -q
}
