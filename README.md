# Description
Given a BFD font file, it outputs a C .h file with bitmap font definition.

# Requiere instalar fontforge

``sudo apt install fontforge``

# Procedure to obtain .h bitmap font definition out of a .ttf

-- Abrir fuente con fontforge.

-- Si "elements" -> "bitmap strikes available" se encuentra greseado, primero haga "file" -> "generate fonts".

-- Seleccionar "bitmap strikes" e ingresar el alto en pixeles deseado. Ejemplo: 16.


