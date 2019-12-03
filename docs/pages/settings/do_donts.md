---
title: Do's & Don'ts
keywords: do dont good bad practice 
sidebar: 3dfier_sidebar
permalink: do_donts.html
---

## DO
- Do validate you YAML configuration using [www.yamllint.com](http://www.yamllint.com)
- Do make sure the input is topologically correct if you expect a watertight output
- Do [*omit_LAS_classes*] of points you do not use, this improves speed a lot and can help overcome configuration mistakes
- Do make sure all [*input_polygons: datasets: lifting*] class has a corresponding [*lifting_options*] class (e.g. [*input_polygons: datasets: lifting: Building*] & [*lifting_options: Building*])

## DON'T
- Don't expect magic
- Don't combine [*simplification*] and [*simplification_tinsimp*] settings, the latter is always preferred
- Don't use lidar [*thinning*] setting for other then testing to improve speed since this is a simple skip amount while reading points
