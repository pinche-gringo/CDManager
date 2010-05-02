<!-- -*-HTML-*- -->
<!--
   Copyright (C) 2004 - 2006, 2010 Markus Schwab (g17m0@users.sourceforge.net)

   This file is part of libYGP.

   libYGP is free software: you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   libYGP is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with libYGP.  If not, see <http://www.gnu.org/licenses/>.
-->

<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN"
        "http://www.w3.org/TR/html4/strict.dtd">

<html>
  <head>
    <title>CDManager - utilidad para administrar discos y pel&iacute;culas</title>
    <meta name="description" content="Documentaci&oacute;n del CD-Manager">
    <meta name="keywords" content="documentacion, documentaci&oacute;n, docu, CD, movie,
                record, manager, disco, pelicula, pel&iacute;cula, CDManager, CD-Manager">

    <meta name="DC.Creator" content="Markus Schwab">
    <meta name="DC.Date" content="2004-12-05">
    <meta name="DC.Rights" content="Copyright (C) 2004 - 2006, 2010; distributed under the GNU Free Documentation License">
  </head>

  <body>
    <h1 align="center">Informaci&oacute;n general sobre el CD-Manager</h1>
    <table width="100%">
      <tr><td align="center" width="33%"><a href="#Intro">Introduci&oacute;n</a></td>
        <td align="center" width="33%"><a href="#Mainwindow">La ventanta principal</a></td>
        <td align="center" width="33%"><a href="#Dialogs">Dialogues</a></p></td></tr>
    </table>
    <hr size=2>

    <h2><a name="Intro"></a>Introduci&oacute;n</h2>

    <p>El CD-Manager es una utiliad para administrar discos y pel&iacute;culas.</p>

    <p>Sus habilitades incluyen exportar la informaci&oacute;n
      guardada a p&aacute;ginas HTML y importar desde canciones en el
      formato MP3 o OGG.</p>

    <p>Este programa se distribuye bajo los condiciones de la GNU Licensia
      General P&uacute;blico, que principalmente expresa lo siguente (v&eacute;ase
      el archivo <tt>COPYING</tt> dentro de la distribuci&oacute;n o
      la <a href="http://www.gnu.org">p&aacute;gina web de GNU
        (http://www.gnu.org)</a> para m&aacute;s detalles):</p>

    <blockquote><em><p>Este programa es software libre; puede
        redistribuirlo y/o modificarlo bajo los t&eacute;rminos de la
        GNU Licencia P&uacute;blica General seg&uacute;n se publica por la
        Free Software Foundation (la Fundaci&oacute;n para el Software
        Libre); tanto de la versi&oacute;n 2 de la Licencia, o (seg&uacute;n su
        elecci&oacute;n) de cualquier versi&oacute;n posterior.</p>

      <p>Este se destribuye con la esperanza de que sea &uacute;til,
        pero SIN NINGUNA GARANT&Iacute;A, ni siquiera la garant&iacute;a
        impl&iacute;cita de COMERCIABILIDAD o CONVENIENCIA PARA UN
        PROP&Oacute;SITO PARTICULAR.  V&eacute;ase la GNU Licensia General
        P&uacute;blico para m&aacute;s detalles).</p></em></blockquote>

    <h2><a name="Mainwindow">Ventana principal</a></h2>
    <p>La ventana principial contiene tres subp&aacute;ginas: Uno por
      los discos, uno por las pel&iacute;culas y uno por los actores.</p>

    <h3>P&aacute;gina Discos</h3>
    <p>Ese p&aacute;gina contiene dos listas; la izquierda containe
      los discos y sus artistas (ordenado por artista y sus discos
      por nombre), la derecha contiene las canciones (ordenado por
      n&uacute;mero de canci&oacte;n.</p>

    <p>El contenido de los valores mostrados se puede cambiar
      &ldquo;inline&rdquo; (significando por hacer clic en una columna
      de la entrada seleccionada).</p>

    <p>Si se selecta un disco, la lista derecha muestra sus canciones,
      cuales se puede cambiar &ldquo;inline&rdquo; tambi&eacute;n.</p>

    <p>Amb&aacute;s listas se puede ordenar por various columnas (con
      hacer click en su t&iacute;tulo.</p>

    <p>A&ntilde;ade entradas con elegir el men&uacute; apropiado bajo
      del men&uacute; <tt>Editar</tt> o con importar la
      informaci&oacute;n guardada en archivos de MP3 o OGG (a
      trav&eacute;s del men&uacute; <tt>Importar informaciones de
      archivos audio</tt> en el men&uacute; <tt>CD</tt>).</p>

    <p>Busca entradas (en la lista activa) con escribir el texto
      buscado. Las teclas <tt>Arriba</tt> y <tt>Bajo</tt> buscan la
      entrada pr&oacute;xima/siguiente.</p>

    <h3>P&aacute;gina Pel&iacute;culas</h3>

    <p>Esa p&aacute;gina contiene las pel&iacute;culas (ordenado por
      su director y a&ntilde;o). Tal cual como los discos se puede
      cambiar la informacion &ldquo;inline&rdquo; (v&eacute;ase arriba).</p>

    <p>Entradas neuves se a&ntilde;ade con elegir la entrada apropiada
      en el men&uacte;  <tt>Editar</tt>.</p>

    <p>Igual a los discos se puede buscar (v&eacute;ase arriba).</p>

    <h3>P&aacute;gina Actores</h3>

    <p>Esa p&aacute;gina contiene los actores (ordenado por defecto por su
      nombre). Tal cual como los discos se puede cambiar la
      informacion &ldquo;inline&rdquo; (v&eacute;ase arriba).</p>

    <p>Entradas neuves se a&ntilde;ade con elegir la entrada apropiada
      en el men&uacute; <tt>Editar</tt>.</p>

    <p>Igual a los discos se puede buscar (v&eacute;ase arriba).</p>

    <p>El men&uacute; <tt>Ver</tt> permita cambiar la vista ordenado
      por actores por pel&iacute;cula o pel&iacute;culas por actor.</p>

    <h2><a name="Dialogs"></a>Dialogues</h2>

    <h3>Conectar</h3>
    <p>Ese dialogue se muestra con elegir <tt>Conectar ...</tt> bajo
      el men&uacute; <tt>CDs</tt>. Permite poner un usario y
      (opcional) su contrase&ntilde;a. Ese informaci&oacute;n
      est&aacute; usado para conectarse a la base de datos.</p>

    <h3>Importar</h3>
    <p>Se muestra con elegir <tt>Importar informaciones de archivos de
      audio ...</tt> bajo el men&uacute; <tt>CDs</tt>. Permite navegar
      el disco duro y elegir archivos. Todos los archivos elegidos
      est&aacute;n analizado por &mdash; depende del tipo &mdash; MP3
      ID3 informaci&oacute;n o comentarios OGG, cuales est&aacute;n
      importados a la base de datos.</p>

    <p>Nota que eso puede resultar en entradas dobles, si los nombres
      est&aacute;n escrito diferente o cortados.</p>

    <h3>Propiedades</h3>
    <p>Se invoca ese dialogue con la entrada <tt>Propiedades ...</tt> del
      men&uacute; <tt>Opciones</tt>.</p>

    <p>Permite cambiar algunos aspectos del programa (relacionado con
      exportar la informaci&oacute;n a documentos HTML o de las
      palabras escpeciales (que se usa por ordenar las entradas)).</p>

    <h2>Autor</h2>
    <p>El programa y la documentaci&oacute;n han sido escrito de Markus
      Schwab.</p>

    <h2>Modo de empleo</h2>
    <p>Principalmente se ejecuta el programa sin ninguna opción:</p>

    <pre>   CDManager</pre>

    <p>Las opciones disponibles se muestra con</p>

    <pre>   CDManager --help
</pre>

    <h2>Cr&eacute;ditos</h2>
    <p>Gracias a la gente de MySQL por su base de datos excelente y su
      biblioteca mysql++.</p>

    <hr noshade="noshade" size="2">
    <address>
      <a href="mailto:g17m0@lycos.com">Markus Schwab (g17m0@lycos.com)</a><br>
    </address>
  </body>
</html>
