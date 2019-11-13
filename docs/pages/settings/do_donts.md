---
title: Do's & Don'ts
keywords: do dont good bad practice 
sidebar: 3dfier_sidebar
permalink: do_donts.html
---

## DO
Do validate you YAML configuration using [www.yamllint.com](http://www.yamllint.com)
Do *omit_LAS_classes* of points you do not use, this improve speed a lot
Do make sure all *input_polygons: datasets: lifting* class has a corresponding *lifting_options* class (e.g. *input_polygons: datasets: lifting: Building* & *lifting_options: Building*)

## DON'T
Don't use *simplification* and *simplification_tinsimp* at the same time
Don't use lidar *thinning* setting for other then testing to improve speed
