# DECT NR+ application template

## Initializing
First, use this template repository to create a new repository on GitHub, using the "Use this template" button.

### Setting up virtual environment
Then, prepare a virtual environment for your project. Replace `<app-name>` with the name of your application.

On linux:
```bash
python -m venv <app-name>/.venv
source <app-name>/.venv/bin/activate
pip install west
```

On windows:
```bat
python -m venv <app-name>/.venv

:: cmd.exe
<app-name>\.venv\Scripts\activate.bat
:: PowerShell
<app-name>\.venv\Scripts\Activate.ps1

pip install west
```

### Cloning the repository
Next, clone the repository using `west` (within the virtual environment) (replace `<repo-owner-name>` and `<app-name>` with the appropriate values):
```bash
west init -m git@github.com:<repo-owner-name>/<app-name>.git --mr dev <app-name>
cd <app-name>
west update
```

### Installing dependencies
In order for `west` to be able to build the project, you need to install the dependencies.
From the `<app-name>` folder, run the following command (within the virtual environment):
```bash
pip install -r nrf/scripts/requirements.txt
pip install -r zephyr/scripts/requirements.txt
```

### Building and running
To build the project, run the following command from the `<app-name>` folder (within the virtual environment):
```bash
west build -p always -b nrf9161dk/nrf9161/ns ./app
```

To flash the project, run the following command (within the virtual environment):
```bash
west flash
```