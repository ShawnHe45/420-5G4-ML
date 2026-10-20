# Atelier ML

## Première partie

Dans le répertoire `apprentissage/`

## Deuxième Partie

Dans le répertoire `analyse_image/`

## Troisième partie

Le repertoire `agent/` contient le code C++ du projet et le devcontainer pour avoir la redirection du server graphique.

## Pour Shawn-Matthew Heng
Si l'environnement virtuel n'est pas créé, écris dans le teminal: py -m venv .venv

Pour activer l'environnement, va au 'Agent host (Local)'
Sinon, écrit ".\.venv\Scripts\Activate.ps1 dans PowerShell" et .venv\Scripts\activate.bat dans cmd

Pour réinstaller les dépendances, écris pip install -r requirements.txt
Sinon, écris pip install requests matplotlib
Si pandas n'est pas inclus, écris python -m pip install pandas ou dans une cellule %pip install pandas
Écrivez pip freeze > requirements.txt pour une réinstallation facile