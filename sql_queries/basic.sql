-- Window용 SQLITE 설치 / 사용
-- 1. sqlite.org/download.html에서 sqlite-tools-win32-x86-3140100.zip down 
-- 2. unzip

-- 1. SQLIte 설치 폴더에서 "SHIFT KEY" + 마우스 오른쪽 버튼 --> cmd 창 뛰우기
-- 2. >sqlite3 test.db

.help
.headers on
.exp on

-- primary key / foreign key enforcement
INSERT INTO DEPT VALUES (10,'TEST','SOMEWHERE');
UPDATE EMP SET DEPTNO = 50 WHERE EMPNO = 7369;

-- NOTE: In order to make foreign key semantics work, 
-- run "PRAGMA FOREIGN_KEYS = ON;" in each session. 

----------------------------------
-- autocommit vs. begin/commit; --
----------------------------------

INSERT INTO DEPT VALUES (50,'TEST','SOMEWHERE');

BEGIN; 
INSERT INTO DEPT VALUES (60, 'TEST2', 'SOMEWHERE2');
-- NOTE: temporary journal file is created!
INSERT INTO EMP VALUES (9999,'SIMON','SECURITY GUARD',7902,date('1980-12-17'),800,NULL,60);
COMMIT; 
-- NOTE: journal file is removed! 

-- Assume that you are familar with the concept of Transaction and ACID. 

-------------------------
-- Concurrency control --
-------------------------
BEGIN; 
INSERT INTO DEPT VALUES (70, 'TEST2', 'SOMEWHERE2');

-- 새 cmd 창을 띄어서, dept 테이블 수행 
BEGIN;
INSERT INTO DEPT VALUES (80, 'TEST2', 'SOMEWHERE2');
-- Why error?
INSERT INTO EMP VALUES (8888,'SIMON','SECURITY GUARD',7902,date('1980-12-17'),800,NULL,60);
-- Why error? lock granularity = file

------------
-- index  --
------------
CREATE INDEX EMP_DNO_IDX ON EMP (DEPTNO);
-- what is 'UNIQUE INDEX'? CREATE UNIQUE INDEX EMP_DNO_IDX ON EMP (DEPTNO);

INSERT INTO EMP VALUES (2222,'Obama','President',7902,date('1980-12-17'),800,NULL,30);
-- How to guarantee atomicity and durability for this autocommiting transaction in SQLite? 
-- SQLite is not taking the traditional WAL protocol and Aries-style recovery. 
-- How many pages are update? 

------------------------
-- query plan explain --
------------------------
.eqp on
SELECT * FROM EMP WHERE EMPNO = 7369;

.exit
