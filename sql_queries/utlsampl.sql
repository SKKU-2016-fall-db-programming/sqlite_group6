---------------------------------------------------------------------
-- Oracle_HOME/RDBMS/ADMIN/UTLSAMPL.SQL을 SQLITE용으로 수정한 버전 --
---------------------------------------------------------------------

-- Copyright (c) 1990, 1996, 1997, 1999, 2001 by Oracle Corporation
-- NAME
-- REM    UTLSAMPL.SQL
--  FUNCTION
--  NOTES
--  MODIFIED
--     menash     02/21/01 -  remove unnecessary users for security reasons
--     gwood      03/23/99 -  make all dates Y2K compliant
--     jbellemo   02/27/97 -  dont connect as system
--     akolk      08/06/96 -  bug 368261: Adding date formats
--     glumpkin   10/21/92 -  Renamed from SQLBLD.SQL 
--     blinden   07/27/92 -  Added primary and foreign keys to EMP and DEPT
--     rlim       04/29/91 -         change char to varchar2 
--     mmoore     04/08/91 -         use unlimited tablespace priv 
--     pritto     04/04/91 -         change SYSDATE to 13-JUL-87 
--   Mendels    12/07/90 - bug 30123;add to_date calls so language independent
-- Rem
-- rem 
-- rem $Header: utlsampl.sql 21-feb-01.18:15:30 menash Exp $ sqlbld.sql 
-- rem 
-- SET TERMOUT OFF
-- SET ECHO OFF

-- rem CONGDON    Invoked in RDBMS at build time.   29-DEC-1988
-- rem OATES:     Created: 16-Feb-83
 
-- DROP USER SCOTT CASCADE;
-- DROP USER ADAMS CASCADE;
-- DROP USER JONES CASCADE;
-- DROP USER CLARK CASCADE;
-- DROP USER BLAKE CASCADE;
-- GRANT CONNECT,RESOURCE,UNLIMITED TABLESPACE TO SCOTT IDENTIFIED BY TIGER;
-- DROP PUBLIC SYNONYM PARTS;

-- CONNECT SCOTT/TIGER

.help
.headers on

DROP TABLE DEPT;
CREATE TABLE DEPT
       (DEPTNO NUMBER(2) PRIMARY KEY,
	DNAME VARCHAR(14),
	LOC VARCHAR(13) ) ;
DROP TABLE EMP;
CREATE TABLE EMP
       (EMPNO NUMBER(4) PRIMARY KEY,
	ENAME VARCHAR(10),
	JOB VARCHAR(9),
	MGR NUMBER(4),
	HIREDATE DATE,
	SAL NUMBER(7,2),
	COMM NUMBER(7,2),
	-- DEPTNO NUMBER(2) CONSTRAINT FK_DEPTNO REFERENCES DEPT);
	DEPTNO NUMBER(2),
        FOREIGN KEY (DEPTNO) REFERENCES DEPT(DEPTNO));

INSERT INTO DEPT VALUES
	(10,'ACCOUNTING','NEW YORK');
INSERT INTO DEPT VALUES (20,'RESEARCH','DALLAS');
INSERT INTO DEPT VALUES
	(30,'SALES','CHICAGO');
INSERT INTO DEPT VALUES
	(40,'OPERATIONS','BOSTON');
-- Oracle to_date() vs. Sqlite date()
-- Oracle: INSERT INTO EMP VALUES (7369,'SMITH','CLERK',7902,date('17-12-1980'),800,NULL,20);
-- SQLite: INSERT INTO EMP VALUES (7369,'SMITH','CLERK',7902,date('1980-12-17'),800, NULL,20);
INSERT INTO EMP VALUES
(7369,'SMITH','CLERK',7902,date('1980-12-17'),800,NULL,20);
INSERT INTO EMP VALUES
(7499,'ALLEN','SALESMAN',7698,date('1981-02-20'),1600,300,30);
INSERT INTO EMP VALUES
(7521,'WARD','SALESMAN',7698,date('1981-02-22'),1250,500,30);
INSERT INTO EMP VALUES
(7566,'JONES','MANAGER',7839,date('1981-02-04'),2975,NULL,20);
INSERT INTO EMP VALUES
(7654,'MARTIN','SALESMAN',7698,date('1981-09-28'),1250,1400,30);
INSERT INTO EMP VALUES
(7698,'BLAKE','MANAGER',7839,date('1981-05-01'),2850,NULL,30);
INSERT INTO EMP VALUES
(7782,'CLARK','MANAGER',7839,date('1981-09-06'),2450,NULL,10);
INSERT INTO EMP VALUES
(7788,'SCOTT','ANALYST',7566,date('1987-07-13'),3000,NULL,20);
INSERT INTO EMP VALUES
(7839,'KING','PRESIDENT',NULL,date('1981-11-17'),5000,NULL,10);
INSERT INTO EMP VALUES
(7844,'TURNER','SALESMAN',7698,date('1981-09-08'),1500,0,30);
INSERT INTO EMP VALUES
(7876,'ADAMS','CLERK',7788,date('1987-07-13'),1100,NULL,20);
INSERT INTO EMP VALUES
(7900,'JAMES','CLERK',7698,date('1981-12-13'),950,NULL,30);
INSERT INTO EMP VALUES
(7902,'FORD','ANALYST',7566,date('1981-12-03'),3000,NULL,20);
INSERT INTO EMP VALUES
(7934,'MILLER','CLERK',7782,date('1982-01-23'),1300,NULL,10);

