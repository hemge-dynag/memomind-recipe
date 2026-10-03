# MemoMind — Recette pas-à-pas (recipe)

Application **appairée** MemoMind : une recette déroulée étape par étape sur les lunettes, avec minuteur par étape, pilotée depuis le téléphone.

## Composition

| Côté | Dossier | Artefact |
| --- | --- | --- |
| Lunettes (plugin natif C) | `glass/recipe/` | `builds/recipe.gmp` |
| Téléphone (plugin Web) | `phone/recipe/` | `builds/recipe-1.0.0.mmpkg` |

## Fonctionnement

- Le téléphone envoie une recette complète (nom + étapes, chaque étape ayant un texte et une durée en minutes). Deux recettes types par langue sont fournies (6 langues).
- Les lunettes affichent l'étape courante (« Étape 2/5 » + texte) : tête haut/bas pour changer d'étape, appui court sur le bouton pour lancer/mettre en pause le minuteur de l'étape. À la fin, un écran « Minuteur terminé ! » s'affiche 3 s et le téléphone est prévenu pour vibrer.
- Le téléphone affiche les étapes et journalise le nombre de recettes envoyées.
- Traduit en 6 langues (FR/EN/ES/IT/DE/ZH) côté lunettes et côté téléphone.

## Protocole binaire

`RECIPE_CHANNEL = 0x5243` ('RC', téléphone → lunettes), `EVENT_CHANNEL = 0x5241` ('RA', lunettes → téléphone), version 1.

- `COMMAND_RECIPE` = `[version, cmd, name_len, name..., count_lo, count_hi, (minutes_lo, minutes_hi, step_len, step...)*]`
- `COMMAND_END` = `[version, cmd, 0, 0, 0]`
- `EVENT_TIMER_DONE` = `[version, event, index_lo, index_hi]`
- `EVENT_STEP` = `[version, event, index_lo, index_hi]`

## Build

Nécessite le SDK officiel MemoMind (`memomind-open/plugin-open-platform`). Déposer `glass/recipe/` dans `GlassSDK/examples/` et `phone/recipe/` dans `PhoneSDK/examples/`, puis lancer `python3 build.py`.

- Lunettes : `python3 build.py glass` → `.gmp`
- Téléphone : `python3 build.py web` → `.mmpkg`

> Note technique : sur la cible RISC-V freestanding, GCC peut replier une copie de littéral en un appel `memcpy` externe malgré `-fno-builtin`. La fonction `format_step_line()` est donc marquée `noinline` et copie son préfixe octet par octet.
