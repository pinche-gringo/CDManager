USE CDMedia;

DROP TABLE MGenres;
CREATE TABLE MGenres
    (id		BIGINT UNSIGNED	NOT NULL,
     genre	VARCHAR(64)	NOT NULL,
     PRIMARY KEY (id));

DROP TABLE Directors;
CREATE TABLE Directors
    (name	VARCHAR (64)	NOT NULL,
     id		BIGINT UNSIGNED	NOT NULL AUTO_INCREMENT,
     born	DATE            NOT NULL DEFAULT 0,
     died	DATE            NOT NULL DEFAULT 0,
     PRIMARY KEY (id));
CREATE INDEX directorNames ON Directors (name);

DROP TABLE Movies;
CREATE TABLE Movies
    (name	VARCHAR (64)	NOT NULL,
     id		BIGINT UNSIGNED	NOT NULL AUTO_INCREMENT,
     year	YEAR            NOT NULL DEFAULT 0,
     director	BIGINT UNSIGNED NOT NULL DEFAULT 0,
     genre      BIGINT UNSIGNED DEFAULT 0,
     PRIMARY KEY (id),
     FOREIGN KEY (genre_id) REFERENCES MGenres(id) ON DELETE SET NULL,
     FOREIGN KEY (director_id) REFERENCES Directors(id) ON DELETE SET NULL);
CREATE INDEX movieNames ON Movies (name);

DROP TABLE Actors;
CREATE TABLE Actors
    (name	VARCHAR (64)	NOT NULL,
     id		BIGINT UNSIGNED	NOT NULL AUTO_INCREMENT,
     born	DATE            NOT NULL DEFAULT 0,
     died	DATE            NOT NULL DEFAULT 0,
     PRIMARY KEY (id));
CREATE INDEX actorNames ON Actors (name);

DROP TABLE ActorsInMovies;
CREATE TABLE ActorsInMovies
    (idActor	BIGINT UNSIGNED	NOT NULL,
     idMovie	BIGINT UNSIGNED	NOT NULL,
     FOREIGN KEY (movie_id) REFERENCES Movies(id) ON DELETE CASCADE,
     FOREIGN KEY (actor_id) REFERENCES Actors(id) ON DELETE CASCADE);

DROP TABLE Genres;
CREATE TABLE Genres
    (id		BIGINT UNSIGNED	NOT NULL,
     genre	VARCHAR(64)	NOT NULL,
     PRIMARY KEY (id));

DROP TABLE Interprets;
CREATE TABLE Interprets
    (name	VARCHAR (64)	NOT NULL,
     id		BIGINT UNSIGNED	NOT NULL AUTO_INCREMENT,
     born	DATE            NOT NULL DEFAULT 0,
     died	DATE            NOT NULL DEFAULT 0,
     PRIMARY KEY (id));
CREATE INDEX interpretNames ON Interprets (name);

DROP TABLE Records;
CREATE TABLE Records
    (name	VARCHAR(64)	NOT NULL,
     id		BIGINT UNSIGNED	NOT NULL AUTO_INCREMENT,
     year	YEAR            NOT NULL DEFAULT 0,
     interpret  BIGINT UNSIGNED	NOT NULL,
     genre	int             NOT NULL DEFAULT 0,
     PRIMARY KEY (id),
     FOREIGN KEY (genre_id) REFERENCES Genres(id) ON DELETE SET NULL,
     FOREIGN KEY (interpret_id) REFERENCES Interprets(id) ON DELETE CASCADE);
CREATE INDEX recordNames ON Records (name);
CREATE INDEX recordInterprets ON Records (interpret);

DROP TABLE Songs;
CREATE TABLE Songs
    (name	VARCHAR(64)	  NOT NULL,
     id		BIGINT UNSIGNED	  NOT NULL AUTO_INCREMENT,
     track      SMALLINT UNSIGNED NOT NULL,
     idRecord   BIGINT UNSIGNED   NOT NULL,
     duration	TIME              NOT NULL,
     genre      BIGINT UNSIGNED   DEFAULT 0,
     PRIMARY KEY (id),
     FOREIGN KEY (genre_id) REFERENCES Genres(id) ON DELETE SET NULL,
     FOREIGN KEY (interpret_id) REFERENCES Records(id) ON DELETE CASCADE);
CREATE INDEX songNames ON Songs (name);
CREATE INDEX songRecord ON Songs (idRecord);

DROP TABLE Words;
CREATE TABLE Words
    (word       VARCHAR(32)     NOT NULL,
     PRIMARY KEY (word));

