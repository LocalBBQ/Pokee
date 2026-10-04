# Yakumo website

This directory contains the static website published through GitHub Pages.
It uses plain HTML, CSS and JavaScript so it works under the repository's
`/Yakumo/` project path without a build step.

Preview it from the repository root:

```sh
python3 -m http.server 4173 --directory site
```

Then open `http://localhost:4173/`. The Pages workflow deploys this directory
after changes reach `main`.
