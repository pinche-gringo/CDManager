USE CDMedia;

DROP TABLE MGenres;
CREATE TABLE MGenres
    (id		BIGINT UNSIGNED	NOT NULL,
     genre	VARCHAR(64)	NOT NULL,
     PRIMARY KEY (id));

DROP TABLE Celebrities;
CREATE TABLE Celebrities
    (name	VARCHAR (64)	NOT NULL,
     id		BIGINT UNSIGNED	NOT NULL AUTO_INCREMENT,
     born	YEAR            NOT NULL DEFAULT 0,
     died	YEAR            NOT NULL DEFAULT 0,
     PRIMARY KEY (id));
CREATE INDEX names ON Celebrities (name);

DROP TABLE Directors;
CREATE TABLE Directors
    (id		BIGINT UNSIGNED	NOT NULL,
     PRIMARY KEY (id),
     FOREIGN KEY (celebrity_id) REFERENCES Celebrities(id) ON DELETE CASCADE);

DROP TABLE Movies;
CREATE TABLE Movies
    (name	VARCHAR (64)	NOT NULL,
     id		BIGINT UNSIGNED	NOT NULL AUTO_INCREMENT,
     year	YEAR            NOT NULL DEFAULT 0,
     director	BIGINT UNSIGNED NOT NULL DEFAULT 0,
     genre      BIGINT UNSIGNED NOT NULL DEFAULT 0,
     type       INT UNSIGNED    NOT NULL DEFAULT 0,
     PRIMARY KEY (id),
     FOREIGN KEY (genre_id) REFERENCES MGenres(id) ON DELETE SET NULL,
     FOREIGN KEY (director_id) REFERENCES Directors(id) ON DELETE SET NULL);
CREATE INDEX movieNames ON Movies (name);

DROP TABLE Actors;
CREATE TABLE Actors
    (id		BIGINT UNSIGNED	NOT NULL,
     PRIMARY KEY (id),
     FOREIGN KEY (celebrity_id) REFERENCES Celebrities(id) ON DELETE CASCADE);

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
    (id		BIGINT UNSIGNED	NOT NULL,
     PRIMARY KEY (id),
     FOREIGN KEY (celebrity_id) REFERENCES Celebrities(id) ON DELETE CASCADE);

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

