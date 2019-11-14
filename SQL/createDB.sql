USE CDMedia;

CREATE TABLE Celebrities
    (name       VARCHAR(64)       NOT NULL,
     id	        BIGINT UNSIGNED   NOT NULL AUTO_INCREMENT,
     born       SMALLINT UNSIGNED NOT NULL DEFAULT 0,
     died       SMALLINT UNSIGNED NOT NULL DEFAULT 0,
     PRIMARY KEY (id), KEY names (name)) DEFAULT CHARSET=utf8;

CREATE TABLE Directors
    (id	        BIGINT UNSIGNED NOT NULL REFERENCES Celebrities(id),
     PRIMARY KEY (id));

CREATE TABLE Films
    (name       VARCHAR(64)     NOT NULL,
     id         BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
     year       YEAR            NOT NULL DEFAULT 0,
     director   BIGINT UNSIGNED NOT NULL DEFAULT 0 REFERENCES Directors(id),
     genre      BIGINT UNSIGNED NOT NULL DEFAULT 0,
     type       INT UNSIGNED    NOT NULL DEFAULT 0,
     languages  VARCHAR(14)     NOT NULL,
     subtitles  VARCHAR(29)     NOT NULL,
     summary    VARCHAR(1024)   NOT NULL,
     image      BLOB            NOT NULL,
     PRIMARY KEY (id), KEY filmNames (name)) DEFAULT CHARSET=utf8;

CREATE TABLE FilmNames
    (id         BIGINT UNSIGNED NOT NULL REFERENCES Films(id),
     name       VARCHAR (64)    NOT NULL,
     language   CHAR(2)         NOT NULL,
     PRIMARY KEY (id, language), KEY filmTranslations (id)) DEFAULT CHARSET=utf8;

CREATE TABLE Actors
    (id	        BIGINT UNSIGNED NOT NULL REFERENCES Celebrities(id),
     PRIMARY KEY (id));

CREATE TABLE ActorsInFilms
    (idActor    BIGINT UNSIGNED NOT NULL REFERENCES Celebrities(id),
     idFilm    BIGINT UNSIGNED NOT NULL REFERENCES Films(id));

CREATE TABLE Interprets
    (id	        BIGINT UNSIGNED NOT NULL REFERENCES Celebrities(id),
     PRIMARY KEY (id));

CREATE TABLE Records
    (name       VARCHAR(64)     NOT NULL,
     id	        BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
     year       YEAR            NOT NULL DEFAULT 0,
     interpret  BIGINT UNSIGNED NOT NULL REFERENCES Interprets(id),
     genre      int             NOT NULL DEFAULT 0,
     PRIMARY KEY (id), KEY recordNames (name), KEY recordInterprets (interpret)) DEFAULT CHARSET=utf8;

CREATE TABLE Songs
    (name       VARCHAR(64)       NOT NULL,
     id	        BIGINT UNSIGNED	  NOT NULL AUTO_INCREMENT,
     track      SMALLINT UNSIGNED NOT NULL,
     idRecord   BIGINT UNSIGNED   NOT NULL REFERENCES Records(id),
     duration   TIME              NOT NULL,
     genre      BIGINT UNSIGNED   DEFAULT 0,
     PRIMARY KEY (id), KEY songNames (name), KEY songRecord (idRecord)) DEFAULT CHARSET=utf8;

CREATE TABLE Words
    (word       VARCHAR(32)     CHARACTER SET binary NOT NULL COLLATE 'binary',
     PRIMARY KEY (word)) DEFAULT CHARSET=utf8;

CREATE TABLE Articles
    (article     VARCHAR(9)    NOT NULL,
     PRIMARY KEY (article)) DEFAULT CHARSET=utf8;
