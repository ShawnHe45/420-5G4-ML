# Plan — Interface graphique CustomTkinter

## Objectif

Ajouter une interface graphique Python avec CustomTkinter au projet C++
`agent/`, en utilisant des bindings pybind11.

## Contraintes

- Ne modifier aucune logique existante dans `book.cpp`, `user.cpp`,
  `library.cpp`, `filemanager.cpp` ou `main.cpp`.
- Ne pas corriger `createBackup()` : l'interface affichera simplement l'erreur
  si un fichier `.backup` existe déjà.
- Le CLI `bibliotheque` doit continuer à fonctionner comme avant.
- Seules modifications C++ : `CMakeLists.txt` et ajout de `bindings.cpp`.

## Architecture cible

```text
agent/
├── CMakeLists.txt
├── bindings.cpp
├── requirements-gui.txt
├── GUI_PLAN.md
└── gui/
    ├── __init__.py
    ├── app.py
    ├── backend.py
    ├── dialogs.py
    └── tables.py
```

## 1. Refactoriser CMakeLists.txt

Le `file(GLOB *.cpp)` actuel inclurait `bindings.cpp` dans l'exécutable CLI. Il
faut séparer le cœur, le CLI et le module Python.

```cmake
set(CORE_SOURCES book.cpp user.cpp library.cpp filemanager.cpp)

add_library(bibliotheque_core STATIC ${CORE_SOURCES})
target_include_directories(bibliotheque_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(bibliotheque_core PRIVATE -Wall -Wextra -Wpedantic)

add_executable(bibliotheque main.cpp)
target_link_libraries(bibliotheque PRIVATE bibliotheque_core)
target_compile_options(bibliotheque PRIVATE -Wall -Wextra -Wpedantic)

option(BUILD_PYTHON_BINDINGS "Build the pybind11 module" OFF)

if(BUILD_PYTHON_BINDINGS)
    find_package(pybind11 CONFIG REQUIRED)

    pybind11_add_module(bibliotheque_py MODULE bindings.cpp)
    target_link_libraries(bibliotheque_py PRIVATE bibliotheque_core)

    set_target_properties(bibliotheque_py PROPERTIES
        OUTPUT_NAME bibliotheque_core
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/python"
    )
endif()
```

Résultat : `build/python/bibliotheque_core.cpython-313-...so`.

## 2. Créer bindings.cpp

Exposer une API Python sûre, avec des copies d'objets au lieu de pointeurs
bruts.

| Classe | API exposée |
|---|---|
| `Book` | `title`, `author`, `isbn`, `is_available`, `borrower_id` |
| `User` | `name`, `user_id`, `borrowed_books` |
| `Library` | `add_book`, `remove_book`, `get_book`, `get_user`, `search_by_title`, `search_by_author`, `all_books`, `available_books`, `all_users`, `check_out`, `return_book`, `stats` |
| `FileManager` | `load`, `save`, `backup` |

- `get_book()` et `get_user()` retournent `std::optional<Book>` et
  `std::optional<User>`.
- Les recherches et listes retournent des `std::vector<Book>` et
  `std::vector<User>` par valeur.
- `stats()` retourne un `py::dict`.
- Inclure `<pybind11/stl.h>`.

Exemple de signature pour éviter d'exposer un pointeur brut :

```cpp
.def("get_book", [](Library& lib, const std::string& isbn)
    -> std::optional<Book> {
        Book* book = lib.findBookByISBN(isbn);
        if (!book) return std::nullopt;
        return *book;
    },
    py::arg("isbn"))
```

## 3. Créer requirements-gui.txt

```text
customtkinter>=5.2,<6
```

Le devcontainer possède déjà Python 3.13, Tkinter 8.6, pybind11 2.13.6 et les
headers Python. CustomTkinter est le seul paquet manquant.

## 4. Créer gui/backend.py

Cette couche encapsule les appels C++ et les validations.

```python
class LibraryError(Exception):
    pass

class Backend:
    def __init__(self, data_dir: Path): ...
    def load(self): ...
    def save(self): ...
    def backup(self): ...
    def books(self): ...
    def available_books(self): ...
    def checked_out_books(self): ...
    def users(self): ...
    def add_book(self, title, author, isbn): ...
    def remove_book(self, isbn): ...
    def add_user(self, name, user_id): ...
    def check_out(self, isbn, user_id): ...
    def return_book(self, isbn): ...
    def search(self, query, by="title"): ...
    def stats(self): ...
```

Validations :

- champs obligatoires non vides ;
- ISBN unique ;
- ID utilisateur unique ;
- livre disponible et utilisateur existant avant emprunt ;
- livre réellement emprunté avant retour ;
- conversion des valeurs de retour C++ en `LibraryError` ;
- capture de l'exception de `backup()` si un backup existe déjà ;
- rechargement de l'état C++ après chaque mutation.

## 5. Créer l'interface gui/

### app.py

- `ctk.CTk`, thème sombre, géométrie `1100x720`.
- Barre supérieure : dossier de données, boutons Sauvegarder et Sauvegarde,
  message de statut.
- `CTkTabview` avec 4 onglets.

### Onglet Livres

- Recherche par titre ou auteur.
- Tableau `ttk.Treeview` : ISBN, Titre, Auteur, Statut, Emprunteur.
- Boutons Ajouter, Supprimer, Actualiser.
- Formulaire d'ajout dans une `CTkToplevel`.

### Onglet Utilisateurs

- Tableau : ID, Nom, Nombre d'emprunts, ISBN empruntés.
- Bouton Ajouter un utilisateur.
- Pas de suppression : `Library` ne possède pas de `removeUser`.

### Onglet Emprunts

- Liste des livres disponibles.
- Liste des utilisateurs.
- Bouton Emprunter.
- Tableau des livres empruntés.
- Bouton Retourner sur la sélection.

### Onglet Statistiques

- Total livres, disponibles, empruntés, utilisateurs.
- Bouton Actualiser.

### Comportement général

- Interface entièrement en français.
- `messagebox` pour les erreurs.
- `refresh_all()` après chaque action réussie.
- Sauvegarde automatique à la fermeture, avec possibilité d'annuler la
  fermeture si la sauvegarde échoue.
- `ttk.Treeview` stylé pour rester cohérent avec CustomTkinter.

## 6. Compilation et lancement

Depuis `agent/` :

```bash
.venv/bin/pip install -r requirements-gui.txt

cmake -S . -B build \
  -DBUILD_PYTHON_BINDINGS=ON \
  -DPYTHON_EXECUTABLE="$PWD/.venv/bin/python" \
  -DPython3_EXECUTABLE="$PWD/.venv/bin/python"

cmake --build build

PYTHONPATH="$PWD/build/python:$PWD" \
  .venv/bin/python -m gui.app --data-dir data
```

Il faudra confirmer que l'affichage graphique fonctionne dans le devcontainer.
Un socket X11 `X7` est présent, mais cela reste à tester avec une fenêtre Tk
minimale.

## 7. Vérifications

1. Le CLI compile et fonctionne toujours :

   ```bash
   cmake -S . -B build
   cmake --build build
   ./build/bibliotheque -d <copie de data>
   ```

2. Le module Python s'importe :

   ```bash
   PYTHONPATH=build/python .venv/bin/python -c "import bibliotheque_core"
   ```

3. Test backend sur une copie de `data/` :

   - chargement ;
   - ajout livre et utilisateur ;
   - emprunt et retour ;
   - sauvegarde et rechargement ;
   - doublons et cas invalides.

4. Test manuel de la GUI :

   - ajout, suppression et recherche de livres ;
   - ajout utilisateur ;
   - emprunt et retour ;
   - statistiques ;
   - sauvegarde et backup ;
   - fermeture avec sauvegarde.

Ne jamais tester directement sur `agent/data/`, qui contient les fixtures
suivies par Git.

## 8. Documentation

Après implémentation, ajouter dans `AGENTS.md` :

- l'option `-DBUILD_PYTHON_BINDINGS=ON` ;
- l'installation de CustomTkinter ;
- la commande de lancement de la GUI.

## Points à confirmer

1. Interface complète en 4 onglets, ou version minimale Livres et Emprunts ?
2. Sauvegarde automatique à la fermeture et bouton manuel, ou uniquement
   bouton manuel ?
3. Dossier `data` fixé au lancement via `--data-dir`, ou modifiable dans
   l'interface ?
