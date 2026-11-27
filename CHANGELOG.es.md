# Cambios

[English](CHANGELOG.md) · **Español** · [日本語](CHANGELOG.ja.md)

## 3.2.0 — 2026-11-27

- **Cambiado:** el tiempo para adivinarla, bajo la barra de fuerza, ahora dice «segundos» por debajo de un minuto,
  agrega «miles de años» entre los siglos y los millones, y por encima de «millones de años» sigue con «miles de
  millones» y «billones de años» antes de «mucho más que la edad del universo», en los tres idiomas.
- **Corregido:** `/accent:0` ahora manda sobre el color guardado en `forja.ini` (antes solo lo hacía un color
  distinto del negro), y `/theme:system` (o `/theme:auto`) pide el tema como Windows, también sobre un tema claro u
  oscuro guardado.
- Los textos en español (ventana, Ayuda, instalador, README, guía) tratan al lector de *tú*, en español neutro.
- `tools/check_options.py` demuestra que dos sorteos nunca dan la misma frase (la lista de la EFF no tiene una
  palabra que sea el comienzo de otra, tampoco con las que llevan guion, como `t-shirt`), así que la entropía que se
  muestra sigue siendo exacta sin separador y sin mayúsculas; ahora además vuelve a separar cada frase en palabras de
  la lista, sea cual sea el separador. El autotest comprueba cada escalón del tiempo para adivinarla.
- **Corregido:** elegir un tiempo en la lista «Borrar portapapeles a los» con el borrado desactivado ahora también
  inicia la cuenta regresiva de la contraseña ya copiada, igual que al activar el borrado.
- **Corregido:** una contraseña copiada se borra aunque otro programa tenga el portapapeles abierto en ese momento:
  Forja reintenta cada segundo, hasta 30 segundos, en vez de rendirse.
- **Corregido (menor):** los íconos de la barra de título y de la barra de tareas se dibujan a la escala del
  monitor donde está la ventana.
- Guías de uso rehechas en los tres idiomas: portada, índice con números de página, figuras numeradas, tablas, la
  ventana con referencias numeradas, todas las opciones, menús, atajos y mensajes, la entropía explicada por tipo, e
  imágenes nuevas. README en inglés y español ampliados (instalación, privacidad, perfiles, teclado, línea de
  comandos); el README japonés quedó a la par.

## 3.1.2 — 2026-11-27

- **Ícono nuevo**: un yunque y un martillo dibujados planos, en crema con contorno negro sobre el cuadrado del color
  elegido, con tres chispas. Reemplaza al grabado de una tarjeta comercial del siglo XVIII y sigue el color elegido en
  *Ver → Tema y color*; también lo usan las imágenes del instalador. El dibujo está en `assets/icon.svg`.

## 3.1.1 — 2026-11-26

- La barra de menús conserva su color después de abrir una lista desplegable (perfil, símbolos, separador, tiempo
  del portapapeles): el pincel con que se pintaba se borraba mientras la barra todavía lo usaba.
- `/out`: una escritura que falla (por ejemplo, con el disco lleno) ahora también deja el archivo vacío, con código
  de salida 2, como cualquier otra falla.
- Actualizaciones: si la búsqueda o la descarga no pueden empezar, la línea de estado ya no se queda en «Buscando
  actualizaciones…» o «Descargando…».
- Ayuda: los niveles de fuerza aparecen completos (menos de 36 bits muy débil, menos de 50 débil, menos de 64
  aceptable, menos de 80 fuerte, 80 o más muy fuerte); la sección de actualizaciones cuenta la pregunta del primer
  arranque y la firma del autor; una sección nueva describe los lectores de pantalla y la escala por monitor. La
  pregunta de actualización dice que las sumas están firmadas por el autor. «Guardar como…» lleva puntos
  suspensivos como los demás ítems del menú.
- Guías de uso: con los colores del propio programa, tomados de su tema predeterminado, con la banda del título y
  el ícono como arriba de la ventana; los niveles de fuerza corregidos; los textos en japonés y en español
  coinciden con el programa.
- README en tres archivos completos — `README.md` (inglés), `README.es.md` (español), `README.ja.md` (japonés) —
  y esta lista de cambios también en español y en japonés.
- Interno: código repetido en varios lugares unido en funciones compartidas (dibujo fuera de pantalla, pinceles en
  caché, ventanas modales, botones predeterminados, clases de ventana, diálogos de tareas, el texto de una
  contraseña), funciones sin uso eliminadas, bloqueos de memoria verificados, la clase de ventana ya no apunta a un
  ícono destruido al arrancar, comentarios revisados.

## 3.1.0 — 2026-11-26

- **El primer arranque pregunta antes de buscar actualizaciones**: sin `CheckUpdates` en `forja.ini` (una
  instalación nueva, o una configuración de la 3.0 o anterior) Forja pregunta una sola vez si puede consultarle a
  GitHub todos los días por una versión nueva, antes de consultar nada. La respuesta se guarda y se puede cambiar en
  el menú Ayuda; cerrar la pregunta no guarda nada y esa vez no se busca. Si lo abre el instalador después de una
  actualización, la búsqueda diaria sigue encendida, sin preguntar.
- **Lectores de pantalla**: el Narrador y los otros lectores ahora nombran cada control que Forja dibuja por su
  cuenta y dicen qué es y cómo está: los modos y los colores como opciones, las fichas y los interruptores como
  casillas encendidas o apagadas, el deslizador con su valor, los desplegables con lo que tienen, los campos de
  número y de texto con su etiqueta, el − y el + de la cantidad, los botones con ícono. Cada fila de la lista se lee
  como su contraseña (la lista la pide cuando un lector la pide; sigue sin guardar ninguna copia), los menús se
  leen, y la línea de estado se lee cuando cambia, sin mover el foco.
- **Escala por monitor** (PerMonitorV2): si la ventana pasa a una pantalla con otra escala, se vuelve a dibujar a
  esa escala — fuentes, íconos, filas y distribución — en vez de estirarse como una imagen, borrosa; Ayuda, Acerca
  de y Tema se vuelven a abrir a esa escala. Una ventana que se abre en una pantalla así toma su escala antes de
  mostrarse.
- Interno: el código de la ventana, un único forja.c de unas 5200 líneas, se dividió en módulos sin ningún cambio
  de comportamiento: forja.c (estado, procedimiento de ventana, entrada), settings.c, theme.c, draw.c, controls.c,
  layout.c, dialogs.c, clipboard.c, filesave.c, updui.c, cli.c y selftest.c, que comparten forja.h, más access.c
  para los lectores de pantalla. La tabla de textos de ui_strings.h se compila una sola vez (en forja.c) en lugar
  de en cada archivo que la incluye.
- La auditoría del código (`tools/audit_sources.py`) vuelve a estar limpia: un comentario de la 3.0.2 tenía una
  letra acentuada.

## 3.0.2 — 2026-11-25

- **Actualizaciones firmadas por el autor**: el `SHA256SUMS.txt` de una versión publicada ahora tiene que llevar la
  firma del autor (`release.sig`, ECDSA P-256, comprobada con la criptografía propia de Windows contra dos claves que
  trae Forja) antes de descargar el instalador. Antes, el instalador se verificaba solo contra sumas de la misma
  página, así que una versión puesta ahí por otra persona se habría instalado sola. Una versión sin firma válida se
  ofrece como página, para bajarla a mano. Cualquiera puede seguir descargando, instalando y compartiendo Forja;
  Forja 3.0.1 y anteriores instalan esta como siempre.

## 3.0.1 — 2026-11-25

- Generar ya no congela la ventana con reglas muy estrictas (por ejemplo, solo dígitos, sin repetir y largo 128
  tardaba 14 segundos y después fallaba). Con «sin repetir» cada carácter se sortea entre todos menos el anterior,
  lo que mantiene cada contraseña válida exactamente igual de probable. Las reglas que dejan tan pocas contraseñas
  que sortear una tardaría demasiado se rechazan enseguida: Generar queda deshabilitado y la línea de estado dice por
  qué, en inglés, japonés y español. El mensaje ya no culpa al generador de números aleatorios de Windows cuando la
  causa eran las reglas.
- Códigos de salida de `/out`: 6 cuando ningún sorteo cumplió las reglas dentro de los intentos permitidos, 3
  cuando falló el generador de números aleatorios (antes los dos eran 3); el archivo queda vacío después de
  cualquier falla.
- Los números aleatorios se piden en bloques, y lo que sobra de un bloque se borra cuando la contraseña está hecha.
- Actualizaciones: cada descarga va a una carpeta nueva con un nombre al azar dentro de `%TEMP%`, el instalador
  queda bloqueado contra escritura desde que se verifica hasta que arranca, y los instaladores y descargas cortadas
  que dejaron actualizaciones anteriores se borran al abrir Forja.
- Apagar «Borrar portapapeles a los» ahora detiene la cuenta regresiva que ya estaba corriendo; encenderlo empieza
  una si el portapapeles todavía tiene la copia de Forja.
- Guardar en archivo escribe todo o nada: una escritura que falla no deja un archivo escrito a medias.
- La ventana se abre donde estaba también cuando la barra de tareas está arriba o a la izquierda (se corría un poco
  cada vez).
- Copiar todas ya no agrega un salto de línea después de la última contraseña.
- Instalador: si falla la desinstalación de Wyrmhold, el instalador lo avisa; el desinstalador también ofrece borrar
  `%APPDATA%\Wyrmhold`.

## 3.0.0 — 2026-11-20

**Wyrmhold ahora es Forja**, un nombre más fácil de recordar: la forja, donde el metal toma forma a golpes de
martillo, como cada contraseña aquí sale del puro azar. El mismo número que Forja 3.0.0 para macOS. El ícono es el
yunque y los martillos de la tarjeta de Elwell and Taylor, herreros de Londres (c. 1766, British Museum, de dominio
público). El repositorio ahora es `forja-windows` (GitHub redirige la dirección vieja).

- Instalar Forja desinstala Wyrmhold en silencio. La primera vez que Forja arranca, `%APPDATA%\Wyrmhold\wyrmhold.ini`
  pasa a ser `%APPDATA%\Forja\forja.ini` (una copia portable toma el `wyrmhold.ini` que tenga al lado como
  `forja.ini`).
- El programa es `forja.exe`; el instalador es `Forja-Setup-<versión>.exe`.
- Compilado con el GCC de MSYS2, como todas las versiones anteriores a la 2.1.0 (la 2.1.0 se había compilado con el
  GCC de WinLibs; el mismo GCC 16.2.0 y el mismo enlazador 2.47, y el programa ya no lleva una biblioteca de hilos
  que no usa).

## 2.1.0 — 2026-11-20

- Portapapeles: lo copiado ahora se borra por defecto a los **2 minutos** (antes eran 30 segundos fijos). Un
  desplegable junto a «Borrar portapapeles a los» ofrece 10 s, 30 s, 1, 2, 5 y 10 minutos; el interruptor sigue
  apagando el borrado. `wyrmhold.ini` guarda `ClearSeconds` (de 5 a 3600) junto a `ClearClipboard`;
  `/clearafter:N[s|m]|off` lo fija para una sesión sin guardarlo.
- Portapapeles: lo copiado se borra solo si el portapapeles sigue teniendo exactamente lo que puso Wyrmhold — el
  número de secuencia o, si no, un SHA-256 del texto (el texto nunca se guarda). El mismo texto vuelto a poner por
  una herramienta de portapapeles sí se borra; cualquier otra cosa copiada después no se toca. Cerrar el programa,
  cerrar sesión o apagar antes de que pase el tiempo lo borra en el acto.
- Actualizaciones que se instalan solas: Ayuda → Buscar actualizaciones, y una búsqueda diaria (Ayuda → Buscar
  actualizaciones todos los días, `CheckUpdates` en el .ini). Una sola pregunta (Actualizar ahora, Ver novedades,
  Más tarde); después el instalador se descarga de la versión publicada en GitHub, se verifica contra su
  `SHA256SUMS.txt` (si no coincide se borra y nunca se ejecuta), corre en silencio conservando las opciones de
  instalación anteriores, y Wyrmhold vuelve a abrirse. A una copia portable se le ofrece la página de descarga.
- `wyrmhold.exe /selftest:<registro>`: el borrado del portapapeles, su configuración y las funciones puras de la
  actualización, con el portapapeles del usuario guardado y restaurado y un .ini descartable; lo corre
  `tools/run_checks.ps1`. `tools/check_clipboard.ps1` ahora corre una copia portable en una carpeta temporal, así
  que nunca toca la configuración real.
- La ayuda, los textos emergentes, el README y las guías de uso describen las dos cosas en inglés, japonés y
  español.
- Pantallas chicas: una ventana nueva toma el tamaño del área de trabajo de la pantalla; las filas que no entran a lo
  ancho pasan a la fila siguiente (ancho mínimo de 560 píxeles lógicos, antes 760); en una ventana baja la banda se
  achica o desaparece, las filas y los espacios se ajustan, y por debajo de eso la parte de arriba de la lista se
  desplaza mientras quedan a la vista al menos tres filas de contraseñas. La línea de estado es una ventana propia.

## 2.0.2 — 2026-11-10

- El instalador crea el acceso directo en el escritorio salvo que lo desmarques (antes venía desmarcado).

## 2.0.1 — 2026-11-10

- La ventana Acerca de muestra la página del proyecto; las guías de uso la enlazan y dicen que Wyrmhold es para
  Windows x64 (no hay versión para ARM).
- `/out`, `/entropy` y `/shot` aceptan nombres de archivo en cualquier idioma (antes se leían en la página de
  códigos ANSI).
- Usar solo la línea de comandos ya no crea una carpeta de configuración vacía en `%APPDATA%`.
- La versión se escribe una sola vez, en `src/version.h`; cada archivo fuente lleva el encabezado GPL-2.0-only.

## 2.0.0 — 2026-11-10

- Nombre nuevo: **Wyrmhold** — del inglés antiguo *wyrm*, dragón, y *hold*, fortaleza: la torre del dragón. Ícono
  nuevo: el dragón alado de Athanasius Kircher (1665) custodiando su torre, en estilo de grabado.
- Tres tipos de contraseña: caracteres, frases (lista grande de la EFF, 3–12 palabras, seis separadores) y PIN (sin
  repetidos, sin seguidos).
- Perfiles: Estándar, Máxima, Sin símbolos, Símbolos mínimos, Corta, Wi-Fi, Frase, PIN.
- Opciones de caracteres: qué tipos usar, juego de símbolos (básicos, todos, mínimos, los tuyos), caracteres a
  excluir, sin repetir seguidos, empezar con letra.
- Generación uniforme entre todas las contraseñas que cumplen las reglas (se descarta la contraseña entera), y la
  entropía exacta, contada con todas las reglas; fuerza en cinco pasos con un tiempo para adivinarla.
- Aspecto nuevo: dibujo suavizado, campos y botones redondeados, una banda con el ícono, menús dibujados por el
  programa.
- Ver → Tema y color: claro, oscuro o como Windows, y doce colores que también pintan el ícono; negro por defecto.
- Instalador en inglés, japonés y español con desinstalador; configuración en `%APPDATA%\Wyrmhold`, o portable con
  un `wyrmhold.ini` al lado del programa.
- Línea de comandos: `/mode`, `/words`, `/classes`, `/symbols`, `/exclude`, `/norepeat`, `/startletter`, `/sep`,
  `/nocap`, `/nonumber`, `/allowruns`, `/preset`, `/entropy`, `/accent`, `/open`.
- Guías de uso reescritas; nueva verificación `tools/check_options.py`.

## 1.0.1 — 2026-11-10

- La ventana Acerca de muestra la licencia (GPL-2.0-only, sin garantía) en inglés, japonés y español.
- Guías de uso: nueva sección 10, Licencia.
- Archivos del proyecto: README (inglés, español, japonés), LICENSE, CHANGELOG, `.gitignore`, `.gitattributes`.

## 1.0.0 — 2026-11-09

- Primera versión: ventana Win32 en C, estática y portable; largo de 4 a 128, de 1 a 50 contraseñas, evitar
  caracteres ambiguos, línea de fuerza, copiar una o todas, guardar en archivo.
- Portapapeles fuera de Win+V y de la nube, borrado a los 30 s o al salir.
- Inglés, japonés y español; tema claro y oscuro; Ayuda y Acerca de.
- Línea de comandos `/out`, `/len`, `/count`, `/ambiguous`, `/lang`, `/theme`, `/shot`.
- Guías de uso en tres idiomas (Word y PDF).
