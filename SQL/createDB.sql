USE CDMedia;

DROP TABLE Movies;
CREATE TABLE Movies
    (name	VARCHAR (64)	NOT NULL,
     id		BIGINT		NOT NULL AUTO_INCREMENT,
     year	INT,
     director	BIGINT,
     PRIMARY KEY (id));
CREATE INDEX movieNames ON Movies (name);

DROP TABLE Directors;
CREATE TABLE Directors
    (name	VARCHAR (64)	NOT NULL,
     id		BIGINT		NOT NULL AUTO_INCREMENT,
     born	DATE,
     died	DATE,
     PRIMARY KEY (id));
CREATE INDEX directorNames ON Directors (name);

DROP TABLE Actors;
CREATE TABLE Actors
    (name	VARCHAR (64)	NOT NULL,
     id		BIGINT		NOT NULL AUTO_INCREMENT,
     born	DATE,
     died	DATE,
     PRIMARY KEY (id));
CREATE INDEX actorNames ON Actors (name);

DROP TABLE ActorsInMovies;
CREATE TABLE ActorsInMovies
    (idActor	BIGINT		NOT NULL,
     idMovie	BIGINT		NOT NULL);

DROP TABLE MGenres;
CREATE TABLE MGenres
    (id		BIGINT		NOT NULL,
     genre	VARCHAR(64)	NOT NULL,
     PRIMARY KEY (id));

DROP TABLE Records;
CREATE TABLE Records
    (name	VARCHAR(64)	NOT NULL,
     id		BIGINT		NOT NULL AUTO_INCREMENT,
     year	INT,
     interpret  BIGINT		NOT NULL,
     genre	int,
     PRIMARY KEY (id));
CREATE INDEX recordNames ON Records (name);

DROP TABLE Songs;
CREATE TABLE Songs
    (name	VARCHAR(64)	NOT NULL,
     id		BIGINT		NOT NULL AUTO_INCREMENT,
     duration	TIME,
     PRIMARY KEY (id));
CREATE INDEX songNames ON Songs (name);

DROP TABLE Interprets;
CREATE TABLE Interprets
    (name	VARCHAR (64)	NOT NULL,
     id		BIGINT		NOT NULL,
     born	DATE,
     died	DATE,
     PRIMARY KEY (id));
CREATE INDEX interpretNames ON Interprets (name);

DROP TABLE Genres;
CREATE TABLE Genres
    (id		BIGINT		NOT NULL,
     genre	VARCHAR(64)	NOT NULL,
     PRIMARY KEY (id));
