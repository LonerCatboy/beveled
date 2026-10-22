<!--
SPDX-FileCopyrightText: 2026 LonerCatboy
SPDX-License-Identifier: CC0-1.0
-->

# Beveled

Tema classico a bevel 3D (stile anni '90/2000) per KDE Plasma 6, disegnato in codice e nitido a
ogni scala.

> **Stato:** in sviluppo iniziale, non ancora utilizzabile.

## Obiettivo

I temi "retro" esistenti si basano su SVG scalati (Kvantum, Aurorae) e mostrano linee sfocate e
artefatti di antialiasing, soprattutto a scale frazionarie (125%, 150%, 175%). Beveled disegna
bevel, cornici e glifi in codice, in pixel fisici: spessori interi, coordinate allineate alla
griglia dello schermo, nessun antialiasing sulle primitive geometriche.

## Componenti previsti

| Componente | Cartella | Stato |
|---|---|---|
| Libreria primitive di disegno | `libbeveled/` | da fare |
| Stile applicazioni (QStyle, anche QML/Kirigami) | `style/` | da fare |
| Decorazione finestre (KDecoration3) | `decoration/` | da fare |
| Schemi colori chiaro e scuro | `colors/` | da fare |
| Stile Plasma, cursori, icone, splash, tema globale | — | in seguito |

## Compilazione

La build avviene in un container [toolbox](https://containertoolbx.org/) dedicato, per non
installare pacchetti di sviluppo sul sistema:

```bash
toolbox run -c kde-dev cmake --preset dev
toolbox run -c kde-dev cmake --build --preset dev
toolbox run -c kde-dev ctest --preset dev
```

## Provare lo stile su una singola applicazione

Da eseguire in una macchina di test (es. una VM), senza installare nulla e senza cambiare le
impostazioni globali:

```bash
QT_PLUGIN_PATH=$PWD/build/bin QT_SCALE_FACTOR=1.5 kwrite -style beveled
```

## Licenze

Progetto conforme a [REUSE](https://reuse.software/):

- codice: GPL-2.0-or-later
- grafica: CC-BY-SA-4.0
- file di configurazione e documentazione: CC0-1.0

I testi completi sono in `LICENSES/`.

## Nota

Progetto indipendente, non affiliato né approvato da Microsoft. Non contiene asset, font o
marchi di Microsoft.
