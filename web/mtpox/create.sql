CREATE USER 'mtpox'@'localhost' IDENTIFIED BY '200000buttcoins';
CREATE DATABASE mtpox;
USE mtpox;
create table plaidcoin_wallets (id varchar(40) not null, amount int(30) not null default 0, primary key(id));
INSERT INTO plaidcoin_wallets VALUES ('phpPhPphpPPPphpcoin', 1333337);
GRANT ALL ON mtpox.* to 'mtpox'@'localhost';

