#!/bin/sh

inkscape -w 64 -h 64 --export-png=Instances@2x.png instance_nobg.svg
inkscape -w 32 -h 32 --export-png=Instances.png instance_nobg.svg

for f in instance_w instance_b; do
  inkscape -w 64 -h 64 --export-png=${f}@2x.png ${f}.svg
  inkscape -w 32 -h 32 --export-png=${f}.png    ${f}.svg
  mv ${f}@2x.png ${f}.png ../../../qt/icons
done
