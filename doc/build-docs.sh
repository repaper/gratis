#!/bin/sh

pandoc -S -N \
    --epub-stylesheet=epub.css \
    --epub-cover-image=images/cog-driving/cover-cog_driver.jpg \
    --epub-metadata=metadata.xml \
    -o cog_driving.epub \
    title-cog_driving.txt \
    cog_driving.md

#pandoc -S -N \
#    cog_driving.md \
#    -o cog_driving.pdf

pandoc -S -N \
    --epub-stylesheet=epub.css \
    --epub-cover-image=images/ek014as014/cover-ek014as014.jpg \
    --epub-metadata=metadata.xml \
    -o ek014as014.epub \
    title-ek014as014.txt \
    ek014as014.md

pandoc -S -N \
    --epub-stylesheet=epub.css \
    --epub-cover-image=images/eg020as012/cover-eg020as012.jpg \
    --epub-metadata=metadata.xml \
    -o eg020as012.epub \
    title-eg020as012.txt \
    eg020as012.md

pandoc -S -N \
    --epub-stylesheet=epub.css \
    --epub-cover-image=images/em027as012/cover-em027as012.jpg \
    --epub-metadata=metadata.xml \
    -o em027as012.epub \
    title-em027as012.txt \
    em027as012.md

pandoc -S -N \
    --epub-stylesheet=epub.css \
    --epub-cover-image=images/extension_board/cover-extension_board.jpg \
    --epub-metadata=metadata.xml \
    -o extension_board.epub \
    title-extension_board.txt \
    extension_board.md