<!-- -*-HTML-*- -->
<!-- $Id: CDManager.html.es,v 1.2 2005/11/12 13:42:54 markus Rel $ -->

<!--
  Copyright (C) 2004, 2005 Markus Schwab (g17m0@lycos.com)

  This is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
-->

<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//ES"
        "http://www.w3.org/TR/html4/strict.dtd">

<html>
  <head>
    <title>CDManager - utiliad para manajar discos y pel&iacute;culas</title>
    <meta name="description" content="Documentaci&oacute;n del CD-Manager">
    <meta name="keywords" content="documentacion, documentaci&oacute;n, docu, CD, movie,
                record, manager, disco, pelicula, pel&iacute;cula, CDManager, CD-Manager">

    <meta name="DC.Creator" content="Markus Schwab">
    <meta name="DC.Date" content="2004-12-05">
    <meta name="DC.Rights" content="Copyright (C) 2004, 2005; distributed under the GNU Free Documentation License">
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

    <p>El CD-Manager es una utiliad para manajar discos y pel&iacute;culas.</p>

    <p>Sus features include exporting the managed media to HTML pages
      and importing records from songs in MP3 format (having an ID3 tag).</p>

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

    <h2><a name="Mainwindow">Main window</a></h2>
    <p>The main window contains a notebook with two pages; one for the
      records and the other for the movies.</p>

    <h3>Record page</h3>
    <p>This page contains two list; the left one holds the stored
      records and their interprets (sorted by interpret; with their
      records sorted by name).</p>

    <p>The content of the displayed values can be changed
      &ldquo;inline&rdquo; (meaning: by clicking in the column of the
      selected line, one wants to change). Usually this opens an entry
      field in which data can be entered , or a list from which the
      values can be selected.</p>

    <p>If a record is selected, the right list shows its songs. Again,
      this list can be edited &ldquo;inline&rdquo;.</p>

    <p>Both lists can be sorted by its various columns (by clicking in
      the title).</p>

    <p>New entries can be added by selecting the appropriate entry in
      the <tt>Edit</tt> menu or by importing the ID3-tag from MP3 files
      (Menu <tt>Import from MP3-info</tt> in the <tt>CD</tt> menu).</p>

    <p>Entries can be searched for (in the active list) by pressing
      <tt>&lt;Ctrl&gt;-F</tt>; within the search-field the keys
      <tt>&lt;Ctrl&gt;-Up</tt> and <tt>&lt;Ctrl&gt;-Down</tt> find the
      previous/next matching entry.</p>

    <h3>Movie page</h3>

    <p>This page contains the movies, sorted by director and (within
      director by year). As with the records, its contents can be
      edited &ldquo;inline&rdquo; (see above).</p>

    <p>New entries can be added by selecting the appropriate entry in
      the <tt>Edit</tt> menu.</p>

    <p>Equal to the record pages, it is possible to searched for
      Entries by pressing <tt>&lt;Ctrl&gt;-F</tt> (see above).</p>

    <h2><a name="Dialogs"></a>Dialogs</h2>

    <h3>Login</h3>
    <p>This dialog is displayed by selecting <tt>Login ...</tt> from
      the <tt>CDs</tt> menu. It is a small window allowing to enter an
      user-id an (optionally) a password. This information is used to
      connect to the database.</p>

    <h3>Import</h3>
    <p>This dialog is displayed by selecting <tt>Import from MP3-info
      ...</tt> from the <tt>CDs</tt> menu. It enables to browse
      through the directory structure and the selection of files. All
      those files are searched for an ID3 tag for MP3 files, which -
      if found - is imported into the database.</p>

    <p>Note that this import might produce double entries, if names
      are differently written or they exceed the the size of the ID3
      information (which is limited to 30/32 characters).</p>

    <h3>Settings</h3>
    <p>The <i>Settings-Dialog</i> is invoked with <tt>Preferences ...</tt> from
      the <tt>Options</tt> menu.</p>

    <p>It allows to change various aspects of the program (related to
    the export of the information to HTML documents).</p>

    <h2>Autor</h2>
    <p>El programa y la documentaci&oacute;n han sido escrito de Markus
      Schwab.</p>

    <h2>Modo de empleo</h2>
    <p>Principalmente se ejecuta el programa sin ninguna opción:</p>

    <pre>   CDManager</pre>

    <p>An overview of the available options can be displayed with</p>

    <pre>   CDManager --help</pre>

    <h2>Credits</h2>
    <p>Thanks to the folks at MySQL for their great database and the
      mysql++ library.</p>

    <hr noshade="noshade" size="2">
    <address>
      <a href="mailto:g17m0@lycos.com">Markus Schwab (g17m0@lycos.com)</a><br>
    </address>
  </body>
</html>
