USE CDMedia;

CREATE TABLE Celebrities
    (name       VARCHAR (64)      NOT NULL,
     id	        BIGINT UNSIGNED   NOT NULL AUTO_INCREMENT,
     born       SMALLINT UNSIGNED NOT NULL DEFAULT 0,
     died       SMALLINT UNSIGNED NOT NULL DEFAULT 0,
     PRIMARY KEY (id));
CREATE INDEX names ON Celebrities (name);

CREATE TABLE Directors
    (id	        BIGINT UNSIGNED NOT NULL REFERENCES Celebrities(id),
     PRIMARY KEY (id));

CREATE TABLE Movies
    (name       VARCHAR (64)    NOT NULL,
     id         BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
     year       YEAR            NOT NULL DEFAULT 0,
     director   BIGINT UNSIGNED NOT NULL DEFAULT 0 REFERENCES Directors(id),
     genre      BIGINT UNSIGNED NOT NULL DEFAULT 0,
     type       INT UNSIGNED    NOT NULL DEFAULT 0,
     languages  VARCHAR(14)     NOT NULL,
     subtitles  VARCHAR(17)     NOT NULL,
     PRIMARY KEY (id));
CREATE INDEX movieNames ON Movies (name);

CREATE TABLE MovieNames
    (id         BIGINT UNSIGNED NOT NULL REFERENCES Movies(id),
     name       VARCHAR (64)    NOT NULL,
     language   CHAR(2)         NOT NULL,
     PRIMARY KEY (id, language));
CREATE INDEX movieTranlations ON MovieNames (id);

CREATE TABLE Actors
    (id	        BIGINT UNSIGNED NOT NULL REFERENCES Celebrities(id),
     PRIMARY KEY (id));

CREATE TABLE ActorsInMovies
    (idActor    BIGINT UNSIGNED NOT NULL REFERENCES Celebrities(id),
     idMovie    BIGINT UNSIGNED NOT NULL REFERENCES Movies(id));

CREATE TABLE Interprets
    (id	        BIGINT UNSIGNED NOT NULL REFERENCES Celebrities(id),
     PRIMARY KEY (id));

CREATE TABLE Records
    (name       VARCHAR(64)     NOT NULL,
     id	        BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
     year       YEAR            NOT NULL DEFAULT 0,
     interpret  BIGINT UNSIGNED NOT NULL REFERENCES Interprets(id),
     genre      int             NOT NULL DEFAULT 0,
     PRIMARY KEY (id));
CREATE INDEX recordNames ON Records (name);
CREATE INDEX recordInterprets ON Records (interpret);

CREATE TABLE Songs
    (name       VARCHAR(64)       NOT NULL,
     id	        BIGINT UNSIGNED	  NOT NULL AUTO_INCREMENT,
     track      SMALLINT UNSIGNED NOT NULL,
     idRecord   BIGINT UNSIGNED   NOT NULL REFERENCES Records(id),
     duration   TIME              NOT NULL,
     genre      BIGINT UNSIGNED   DEFAULT 0,
     PRIMARY KEY (id));
CREATE INDEX songNames ON Songs (name);
CREATE INDEX songRecord ON Songs (idRecord);

CREATE TABLE Words
    (word       VARCHAR(32)     CHARACTER SET binary NOT NULL COLLATE 'binary',
     PRIMARY KEY (word));

CREATE TABLE Articles
    (article     VARCHAR(9)    NOT NULL,
     PRIMARY KEY (article));
