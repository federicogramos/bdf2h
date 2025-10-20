# Description
Given a BFD font file, it outputs a C .h file with bitmap font definition.

# Requires fontforge intallation

If you want to start from a ttf font, you can obtain the .bdf file using fontforge. In ubuntu or mac, fontforge can do the trick.

For ubuntu:

``sudo apt install fontforge``

# Procedure to obtain .h bitmap font definition out of a .ttf

-- Abrir fuente con fontforge.

### Disminuir la cantidad de caracteres a unicamente ascii de 256.

-- ``encoding > reencode > custom`` (windows latin ansi tambien es una opcion posible).

-- Seleccionar chars mas alla de 0xFF y elminarlos haciendo (``edit > clear``). Debe asegurarse que no quede nada, luego ``encoding > remove unused slots``. En ocasiones tambien es necesario hacer ``encoding > detach glymphs`` (con las seleccionadas unicamente).

-- Si no se eliminan los chars que se quieren eliminar, revisar nuevamente el encoding que se encuentre en "custom" puesto que con algunas acciones puede restaurarse uno diferente.

-- Revise que los caracteres ascii esten en sus posiciones, 'a' en 0x61, etc.

### Rellenar aquellos caracteres donde se vea una "X"

-- Todos los chars tienen que tener encoding, por lo que seleccionar todas y hacer:
``encoding > force encoding > windows latin ansi``

-- asegurarse que todos los chars esten completos (aquellos que estan marcados con una cruz no seran generados en el bitmap font y genera un arreglo de menos de 256 caractere). Por ello, para rellenar todos los caracteres puede copiar el caracter "espacio" y pegarlo donde necesite.

-- Recomendacion: guardar el archivo en formato propio del fontforge (.sfd) como backup luego de haber reducido a 256 los chars y rellenar los huecos. Una vez guardado, cerrar fontforge y volver a abrirlo usando el recien guardado .sfd y volver a revisar que ningun caracter tenga "X" (vacío). Llegado el caso, rellenar ese char.

### Generar bitmap

-- Si ``elements > bitmap strikes available`` se encuentra griseado, primero haga ``file > generate fonts``.

-- Seleccionar "bitmap strikes" e ingresar el alto en pixeles deseado. Ejemplo: 16.

Sugerencia: a veces una fuente pasada a bitmap usando bitmap-strike = 16 queda con ancho > 8px pero si prueba usar otro valor de bitmap-strike < 16 queda con el valor ancho = 8px que es bastante cómodo para generar los barridos de cada caracter. 

Con la fuente ya generada como bitmap puede, si es que quiere, hacer una edicion personalizada de caracteres ajustando pixeles.

### Guardar bdf y generar .h

-- ``file > generate fonts``, y en el menu cambiar la extension del arhivo a guardar por .bdf

-- Antes de guardar, seleccionar 16 como altura (o la que deba ser) todas las veces que deba indicarse el tamano.

-- Va a generar un archivo con nombre xxxxx-16.bdf

-- Por último: use el programa bdf2h.
``./bdf2h -i xxxxx-16.bdf -o output.h --hex``

### Ajustes finales

Es posible hacer ajustes del header generado directamente abriendo el archivo .bdf con un editor de texto y ajustando los parametros que alli define.