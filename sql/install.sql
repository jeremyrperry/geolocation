# ************************************************************
# Sequel Pro SQL dump
# Version 4096
#
# http://www.sequelpro.com/
# http://code.google.com/p/sequel-pro/
#
# Host: localhost (MySQL 5.5.28)
# Database: web_services
# Generation Time: 2013-10-23 19:27:29 +0000
# ************************************************************


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;


# Dump of table geolocation_api_track
# ------------------------------------------------------------

DROP TABLE IF EXISTS `geolocation_api_track`;

CREATE TABLE `geolocation_api_track` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `api_val_id` int(11) NOT NULL,
  `insert_count` int(11) NOT NULL,
  `timestamp` int(20) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;



# Dump of table geolocation_api_values
# ------------------------------------------------------------

DROP TABLE IF EXISTS `geolocation_api_values`;

CREATE TABLE `geolocation_api_values` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(255) NOT NULL DEFAULT '',
  `description` varchar(255) NOT NULL DEFAULT '',
  `api_key` varchar(255) NOT NULL DEFAULT '',
  `limit_qty` int(11) DEFAULT NULL,
  `limit_time_unit` varchar(255) DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `name` (`name`),
  UNIQUE KEY `api_key` (`description`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;



# Dump of table geolocation_cities
# ------------------------------------------------------------

DROP TABLE IF EXISTS `geolocation_cities`;

CREATE TABLE `geolocation_cities` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `country` varchar(255) CHARACTER SET latin1 NOT NULL DEFAULT '',
  `state_region` varchar(255) CHARACTER SET latin1 DEFAULT NULL,
  `city` varchar(255) CHARACTER SET latin1 DEFAULT NULL,
  `postal_code` varchar(255) CHARACTER SET latin1 DEFAULT NULL,
  `lat` float NOT NULL,
  `lng` float NOT NULL,
  `dma_code` int(11) DEFAULT NULL,
  `area_code` int(11) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;



# Dump of table geolocation_dma
# ------------------------------------------------------------

DROP TABLE IF EXISTS `geolocation_dma`;

CREATE TABLE `geolocation_dma` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `dma_code` varchar(100) NOT NULL DEFAULT '',
  `region_name` varchar(100) NOT NULL DEFAULT '',
  `lng` float NOT NULL,
  `lat` float NOT NULL,
  `adperc` varchar(100) NOT NULL,
  `tvperc` varchar(100) NOT NULL DEFAULT '',
  `cableperc` varchar(100) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`),
  UNIQUE KEY `dma_code` (`dma_code`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

LOCK TABLES `geolocation_dma` WRITE;
/*!40000 ALTER TABLE `geolocation_dma` DISABLE KEYS */;

INSERT INTO `geolocation_dma` (`id`, `dma_code`, `region_name`, `lng`, `lat`, `adperc`, `tvperc`, `cableperc`)
VALUES
	(1,'662','Abilene-Sweetwater, TX',-99.8294,32.4043,'51.8','89.2','38.2'),
	(2,'525','Albany, GA',-83.7121,31.528,'42.6','92.7','50.9'),
	(3,'532','Albany-Schenectady-Troy, NY',-73.8439,43.0493,'17.2','92.1','75.3'),
	(4,'790','Albuquerque-Santa Fe, NM',-106.527,34.4953,'44.8','83.3','38.9'),
	(5,'644','Alexandria, LA',-92.618,31.3226,'38.8','92.4','54.4'),
	(6,'583','Alpena, MI',-83.5734,44.8563,'32.6','90.3','58'),
	(7,'634','Amarillo, TX',-102.063,35.4797,'47','91','44.6'),
	(8,'524','Atlanta, GA',-84.3335,33.9254,'38.7','93.9','55.7'),
	(9,'520','Augusta, GA',-81.9064,33.2388,'34.1','89.8','56.3'),
	(10,'635','Austin, TX',-98.0273,30.3314,'27.3','87.9','61.6'),
	(11,'800','Bakersfield, CA',-119.055,35.291,'35.8','85.7','50.5'),
	(12,'512','Baltimore, MD',-76.5065,38.9725,'21.6','95.1','74.1'),
	(13,'537','Bangor, ME',-68.7511,45.3357,'44.5','84.6','40.5'),
	(14,'716','Baton Rouge, LA',-91.1409,30.4278,'25.5','94.2','69.4'),
	(15,'692','Beaumont-Port Arthur, TX',-94.133,30.3724,'42.2','92.1','50.4'),
	(16,'821','Bend, OR',-120.95,44.0016,'22.8','82.6','60.5'),
	(17,'756','Billings, MT',-108.195,45.8919,'34.2','85.5','51.9'),
	(18,'746','Biloxi-Gulfport, MS',-88.8684,30.6511,'25.2','92.3','68.1'),
	(19,'502','Binghamton, NY',-75.4932,42.2963,'19.3','91.4','72.8'),
	(20,'630','Birmingham (Anniston and Tuscaloosa), AL',-86.8696,33.5031,'45.3','93.4','48.7'),
	(21,'559','Bluefield-Beckley-Oak Hill, WV',-80.9782,37.6002,'33.4','95.7','63.1'),
	(22,'757','Boise, ID',-117.023,43.632,'46.2','70.7','24.7'),
	(23,'506','Boston, MA (Manchester, NH)',-71.4605,42.501,'12.8','98.8','87.2'),
	(24,'736','Bowling Green, KY',-85.9923,37.0342,'20.6','90.8','71'),
	(25,'514','Buffalo, NY',-78.68,42.4237,'30.7','92.3','62.2'),
	(26,'523','Burlington, VT-Plattsburgh, NY',-73.0369,44.0713,'38.7','90.8','52.5'),
	(27,'754','Butte-Bozeman, MT',-112.361,45.9791,'39.6','81.9','42.5'),
	(28,'767','Casper-Riverton, WY',-107.473,43.4129,'31.6','91.3','60.3'),
	(29,'637','Cedar Rapids-Waterloo-Iowa City & Dubuque, IA',-91.7494,42.3312,'31.6','86.5','55.4'),
	(30,'648','Champaign & Springfield-Decatur, IL',-89.0627,39.9607,'35.2','92.1','57.4'),
	(31,'519','Charleston, SC',-80.0423,33.1812,'28.9','92','63.7'),
	(32,'564','Charleston-Huntington, WV',-82.0395,38.3762,'41.2','94.4','53.7'),
	(33,'517','Charlotte, NC',-80.7695,35.4772,'34.8','92.6','58.5'),
	(34,'584','Charlottesville, VA',-78.4611,38.1802,'35.5','86.2','51.3'),
	(35,'575','Chattanooga, TN',-84.7885,35.2041,'33.2','91.2','58.6'),
	(36,'759','Cheyenne, WY-Scottsbluff, NE',-104.321,41.8049,'30.8','92.2','61.6'),
	(37,'602','Chicago, IL',-87.8456,41.6151,'28.2','89.2','62'),
	(38,'868','Chico-Redding, CA',-122.35,40.336,'53.6','85.4','32.3'),
	(39,'515','Cincinnati, OH',-84.3579,39.0339,'27.4','85.7','58.7'),
	(40,'598','Clarksburg-Weston, WV',-80.3091,38.9333,'42','95.4','54.2'),
	(41,'510','Cleveland-Akron (Canton), OH',-81.7112,41.0959,'24.2','94.2','70.2'),
	(42,'752','Colorado Springs-Pueblo, CO',-104.027,38.0614,'43.5','86.5','43.6'),
	(43,'546','Columbia, SC',-80.9102,33.8964,'38.8','88.6','50.5'),
	(44,'604','Columbia-Jefferson City, MO',-92.268,38.8629,'52.5','85.6','33.4'),
	(45,'522','Columbus, GA',-84.8354,32.297,'27.2','93.2','66.6'),
	(46,'535','Columbus, OH',-82.7342,39.972,'22.4','89.4','67.1'),
	(47,'673','Columbus-Tupelo-West Point, MS',-88.939,33.9624,'59.3','90','31.6'),
	(48,'600','Corpus Christi, TX',-97.8595,27.6934,'32','90.8','59.4'),
	(49,'623','Dallas-Ft. Worth, TX',-97.0989,32.7025,'36.2','83.9','48.5'),
	(50,'682','Davenport, IA-Rock Island-Moline, IL',-90.4413,41.5681,'34','87.7','54.1'),
	(51,'542','Dayton, OH',-84.3548,40.1392,'24.7','85.4','60.8'),
	(52,'751','Denver, CO',-108.925,40.9981,'41.1','89.6','48.9'),
	(53,'679','Des Moines-Ames, IA',-93.6951,41.8823,'38.4','83','45.2'),
	(54,'505','Detroit, MI',-83.2884,42.707,'20.6','89.9','69.8'),
	(55,'606','Dothan, AL',-85.4124,31.243,'37.5','91.2','54.3'),
	(56,'676','Duluth, MN-Superior, WI',-91.7104,47.1696,'44.6','81.6','37.3'),
	(57,'765','El Paso, TX',-105.662,31.8409,'31.1','72','41.6'),
	(58,'565','Elmira, NY',-77.1441,42.0595,'26.6','94.6','68.5'),
	(59,'516','Erie, PA',-79.7176,41.8794,'32.8','86.9','54.8'),
	(60,'801','Eugene, OR',-123.126,43.6938,'35.4','86.2','51.3'),
	(61,'802','Eureka, CA',-123.909,41.0014,'26.4','83.9','58.6'),
	(62,'649','Evansville, IN',-87.5662,37.8233,'40.9','89.9','49.3'),
	(63,'724','Fargo-Valley City, ND',-97.1384,47.6598,'34.8','89.2','54.9'),
	(64,'513','Flint-Saginaw-Bay City, MI',-83.8473,43.6439,'31','90.2','59.6'),
	(65,'866','Fresno-Visalia, CA',-119.613,36.8466,'46.4','80.4','34.7'),
	(66,'571','Ft. Myers-Naples, FL',-81.6231,26.572,'33.5','94.9','62.5'),
	(67,'670','Ft. Smith-Fayetteville-Springdale-Rogers, AR',-94.2029,35.5032,'38.7','89.5','51.1'),
	(68,'509','Ft. Wayne, IN',-85.1431,41.0335,'34.3','79','45.1'),
	(69,'592','Gainesville, FL',-82.7295,29.4735,'33.9','89.8','56.6'),
	(70,'798','Glendive, MT',-105.066,46.8364,'26','93.8','68.6'),
	(71,'773','Grand Junction-Montrose, CO',-108.215,38.4257,'32.5','84.9','52.8'),
	(72,'563','Grand Rapids-Kalamazoo-Battle Creek, MI',-85.6241,42.7892,'31.6','86.4','55.2'),
	(73,'755','Great Falls, MT',-109.936,47.8388,'41.2','88','47.3'),
	(74,'658','Green Bay-Appleton, WI',-88.2017,44.7645,'30.6','80','49.7'),
	(75,'518','Greensboro-High Point-Winston Salem, NC',-80.4077,35.9947,'33.5','91.9','58.9'),
	(76,'545','Greenville-New Bern-Washington, NC',-76.9754,35.3422,'35.7','89','53.7'),
	(77,'567','Greenville-Spartanburg, SC-Asheville, NC-Anderson,SC',-82.6977,35.0527,'47.2','92.8','46'),
	(78,'647','Greenwood-Greenville, MS',-90.4836,33.5851,'35.1','92.8','58.9'),
	(79,'636','Harlingen-Weslaco-Brownsville-McAllen, TX',-98.1577,26.3119,'29.3','69.7','41'),
	(80,'566','Harrisburg-Lancaster-Lebanon-York, PA',-76.882,40.2836,'25','92.5','68'),
	(81,'569','Harrisonburg, VA',-79.0679,38.4232,'33.9','91.8','58.4'),
	(82,'533','Hartford & New Haven, CT',-72.6594,41.6127,'15.1','98.3','84.1'),
	(83,'710','Hattiesburg-Laurel, MS',-89.245,31.567,'42.9','90.7','49.3'),
	(84,'766','Helena, MT',-112.332,47.1878,'28.3','82.6','54.8'),
	(85,'618','Houston, TX',-95.6464,29.6061,'27.6','83','56.2'),
	(86,'691','Huntsville-Decatur (Florence), AL',-86.8583,34.7954,'36.6','93.1','56.8'),
	(87,'758','Idaho Falls-Pocatello, ID',-112.677,43.6417,'47.7','79.8','32.7'),
	(88,'527','Indianapolis, IN',-86.1652,39.8429,'28.9','86.4','58.1'),
	(89,'718','Jackson, MS',-90.4857,32.1874,'51.5','91.6','40.7'),
	(90,'639','Jackson, TN',-88.5302,35.5708,'32.6','91.6','59.7'),
	(91,'561','Jacksonville, FL',-82.0066,30.4276,'34.3','92.1','58.4'),
	(92,'574','Johnstown-Altoona, PA',-78.3282,40.6731,'36.6','95.9','59.7'),
	(93,'734','Jonesboro, AR',-91.1295,36.0989,'31.8','91.3','60.5'),
	(94,'603','Joplin, MO-Pittsburg, KS',-95.0098,37.2696,'45.5','81','36.1'),
	(95,'616','Kansas City, MO',-94.209,39.3042,'24.7','87.2','62.7'),
	(96,'557','Knoxville, TN',-84.0422,36.1127,'34.9','92.3','57.9'),
	(97,'702','La Crosse-Eau Claire, WI',-91.1973,44.3132,'32.9','86.5','54'),
	(98,'582','Lafayette, IN',-87.1134,40.3452,'27.3','93.1','66.4'),
	(99,'642','Lafayette, LA',-92.112,30.238,'34.7','92.5','58.3'),
	(100,'643','Lake Charles, LA',-93.262,30.2391,'29.3','92.5','64.7'),
	(101,'551','Lansing, MI',-84.6026,42.4075,'34.2','86.9','53.1'),
	(102,'749','Laredo, TX',-99.5051,27.3882,'28.6','80.7','53.2'),
	(103,'839','Las Vegas, NV',-116.122,37.0828,'29.8','91.9','63.1'),
	(104,'541','Lexington, KY',-83.993,37.5849,'42.8','91.6','49'),
	(105,'558','Lima, OH',-84.1388,40.7826,'18.8','92.4','73.8'),
	(106,'722','Lincoln & Hastings-Kearney, NE',-99.2576,41.2828,'40.2','89.6','49.8'),
	(107,'693','Little Rock-Pine Bluff, AR',-92.6704,34.6471,'48.7','91.6','43.3'),
	(108,'803','Los Angeles, CA',-116.859,35.1328,'35.3','87.1','52.6'),
	(109,'529','Louisville, KY',-85.775,38.1525,'28.2','89.8','62.1'),
	(110,'651','Lubbock, TX',-101.791,33.4181,'39.4','83.3','44.6'),
	(111,'503','Macon, GA',-83.319,32.6219,'44.3','93.4','49.6'),
	(112,'669','Madison, WI',-89.9628,43.3707,'32.4','81.6','49.6'),
	(113,'737','Mankato, MN',-94.4376,43.8766,'22.6','88.9','66.8'),
	(114,'553','Marquette, MI',-87.8765,46.8707,'28.9','95','66.5'),
	(115,'813','Medford-Klamath Falls, OR',-121.963,42.3097,'48.1','87.6','39.9'),
	(116,'640','Memphis, TN',-89.758,35.1946,'41.3','90.8','50'),
	(117,'711','Meridian, MS',-88.5818,32.3469,'53.7','87.1','34.6'),
	(118,'528','Miami-Fort Lauderdale, FL',-80.9406,25.439,'26.5','93.9','69.9'),
	(119,'617','Milwaukee, WI',-88.3582,43.1921,'17.6','77.7','61'),
	(120,'613','Minneapolis-St. Paul, MN',-94.199,46.0198,'27.5','81.9','54.9'),
	(121,'687','Minot-Bismarck-Dickinson(Williston), ND',-102.718,46.8732,'30.2','90.2','60.4'),
	(122,'762','Missoula, MT',-114.464,47.2291,'46.5','83.3','37.1'),
	(123,'686','Mobile, AL-Pensacola (Ft. Walton Beach), FL',-87.6152,31.1061,'39.8','91','51.7'),
	(124,'628','Monroe, LA-El Dorado, AR',-91.9322,32.1843,'47','92.4','46'),
	(125,'828','Monterey-Salinas, CA',-121.266,36.5375,'38.5','90.1','52.8'),
	(126,'698','Montgomery-Selma, AL',-86.7676,32.0507,'30.4','93.9','64'),
	(127,'570','Myrtle Beach-Florence, SC',-79.4198,34.2988,'25.2','91.3','66.6'),
	(128,'659','Nashville, TN',-86.6311,36.0726,'36.5','92.5','56.7'),
	(129,'622','New Orleans, LA',-90.0825,29.9723,'33.5','92.2','60.2'),
	(130,'501','New York, NY',-73.6073,40.8397,'13.8','96.8','84.2'),
	(131,'544','Norfolk-Portsmouth-Newport News, VA',-76.3721,36.6083,'23.1','92.1','69.5'),
	(132,'740','North Platte, NE',-100.836,41.395,'30.4','91','61.9'),
	(133,'633','Odessa-Midland, TX',-103.077,30.7815,'32.4','94.2','62.8'),
	(134,'650','Oklahoma City, OK',-97.992,35.6673,'30.8','84','53.4'),
	(135,'652','Omaha, NE',-96.1507,41.1056,'21.7','89.1','67.8'),
	(136,'534','Orlando-Daytona Beach-Melbourne, FL',-81.4963,28.6562,'26.2','94.2','69.1'),
	(137,'631','Ottumwa, IA-Kirksville, MO',-92.5451,40.3841,'38.8','87','48.6'),
	(138,'632','Paducah, KY-Cape Girardeau, MO-Harrisburg, IL',-89.4973,37.2353,'55.2','91.2','36.6'),
	(139,'804','Palm Springs, CA',-116.235,33.7299,'22.6','96.3','75'),
	(140,'656','Panama City, FL',-85.3696,30.2937,'27.8','90','63'),
	(141,'597','Parkersburg, WV',-81.4306,39.3361,'23.4','94.4','72'),
	(142,'675','Peoria-Bloomington, IL',-89.3432,40.6826,'30.4','89.4','59.5'),
	(143,'504','Philadelphia, PA',-75.3875,39.8996,'17','94.7','79.1'),
	(144,'753','Phoenix, AZ',-111.9,34.7134,'38.8','86.5','48.2'),
	(145,'508','Pittsburgh, PA',-79.659,40.4097,'23.5','94.5','71.4'),
	(146,'820','Portland, OR',-120.45,44.1913,'31','87.6','56.8'),
	(147,'500','Portland-Auburn, ME',-70.183,44.3667,'23.7','91.3','67.9'),
	(148,'552','Presque Isle, ME',-68.9026,46.5196,'35.9','91','56.2'),
	(149,'521','Providence, RI-New Bedford, MA',-71.3513,41.6209,'9.5','93.9','84.8'),
	(150,'717','Quincy, IL-Hannibal, MO-Keokuk, IA',-91.1726,40.0661,'46.4','86.9','41'),
	(151,'560','Raleigh-Durham (Fayetteville), NC',-78.4213,35.7225,'35.1','89.9','55.6'),
	(152,'764','Rapid City, SD',-104.479,43.7649,'28.3','89.7','61.9'),
	(153,'811','Reno, NV',-118.959,39.4852,'43','89.3','46.8'),
	(154,'556','Richmond-Petersburg, VA',-77.535,37.4686,'35.6','91.5','56.1'),
	(155,'573','Roanoke-Lynchburg, VA',-79.9114,37.6409,'49.8','90.1','40.6'),
	(156,'538','Rochester, MN-Mason City, IA-Austin, MN',-77.3872,42.9138,'18.6','87.6','69.5'),
	(157,'611','Rochester, NY',-92.851,43.5522,'32.4','89','57'),
	(158,'610','Rockford, IL',-89.3161,42.0454,'30.4','89.6','59.7'),
	(159,'862','Sacramento-Stockton-Modesto, CA',-120.991,38.7885,'39.6','90.9','52.2'),
	(160,'576','Salisbury, MD',-75.5486,38.4438,'19.5','93.5','74.5'),
	(161,'770','Salt Lake City, UT',-112.264,40.2319,'40','82.5','42.7'),
	(162,'661','San Angelo, TX',-100.741,31.1869,'39','95.4','57.3'),
	(163,'641','San Antonio, TX',-99.1607,29.2651,'29.4','87.8','58.5'),
	(164,'825','San Diego, CA',-116.84,33.0196,'13.7','93.8','80.6'),
	(165,'807','San Francisco-Oakland-San Jose, CA',-122.619,38.4477,'28.3','92.6','65'),
	(166,'855','Santa Barbara-Santa Maria-San Luis Obispo, CA',-120.41,34.8447,'34.8','92.4','58.4'),
	(167,'507','Savannah, GA',-81.6331,32.1682,'37.3','95.2','58.7'),
	(168,'819','Seattle-Tacoma, WA',-121.842,47.6212,'21.3','93.8','72.8'),
	(169,'657','Sherman, TX-Ada, OK',-96.25,34.1815,'47.8','86.9','39.9'),
	(170,'612','Shreveport, LA',-94.0138,32.8209,'55.2','91.7','37.3'),
	(171,'624','Sioux City, IA',-96.3763,42.6215,'32.8','86.5','54.1'),
	(172,'725','Sioux Falls (Mitchell), SD',-98.7352,44.0134,'26.5','89.5','63.3'),
	(173,'588','South Bend-Elkhart, IN',-86.0623,41.5765,'39.6','79.9','40.8'),
	(174,'881','Spokane, WA',-117.604,47.0399,'45','85.2','40.5'),
	(175,'619','Springfield, MO',-92.547,37.1293,'51.7','81.9','30.8'),
	(176,'543','Springfield-Holyoke, MA',-72.6017,42.3857,'14','94.8','81.5'),
	(177,'638','St. Joseph, MO',-94.8792,40.0553,'29.4','86.6','57.8'),
	(178,'609','St. Louis, MO',-90.1693,38.4343,'39.7','89.9','50.5'),
	(179,'555','Syracuse, NY',-76.1014,42.985,'16.7','91.1','74.9'),
	(180,'530','Tallahassee, FL-Thomasville, GA',-83.6757,30.4623,'38.6','93','55.1'),
	(181,'539','Tampa-St. Petersburg (Sarasota), FL',-81.8974,27.9989,'14.7','92.8','78.9'),
	(182,'581','Terre Haute, IN',-87.6904,39.2814,'50.6','88.6','38.8'),
	(183,'547','Toledo, OH',-83.7585,41.385,'25.1','88.4','63.6'),
	(184,'605','Topeka, KS',-96.5557,39.0202,'31.3','88.8','58'),
	(185,'540','Traverse City-Cadillac, MI',-84.952,45.1192,'41.7','86.9','45.5'),
	(186,'531','Tri-Cities, TN-VA',-82.4685,36.7325,'37.5','93.6','56.5'),
	(187,'789','Tucson (Sierra Vista), AZ',-111.191,31.923,'39.3','87.1','48.3'),
	(188,'671','Tulsa, OK',-95.7686,35.99,'37.6','87.7','50.1'),
	(189,'760','Twin Falls, ID',-114.044,42.9945,'47.2','80.9','34.4'),
	(190,'709','Tyler-Longview(Lufkin & Nacogdoches), TX',-94.8807,32.107,'53.7','91.3','38'),
	(191,'526','Utica, NY',-75.0263,43.2055,'17.4','92.5','75.7'),
	(192,'626','Victoria, TX',-96.9738,28.8057,'30.9','90.7','60.6'),
	(193,'625','Waco-Temple-Bryan, TX',-97.3522,31.0795,'38.8','91.3','52.9'),
	(194,'511','Washington, DC (Hagerstown, MD)',-77.9044,39.1277,'25.1','94.1','69.7'),
	(195,'549','Watertown, NY',-75.4894,44.2172,'23.8','93','69.8'),
	(196,'705','Wausau-Rhinelander, WI',-89.6753,44.9711,'38.5','84.1','46'),
	(197,'548','West Palm Beach-Ft. Pierce, FL',-80.6272,27.0967,'26.8','96.8','71.6'),
	(198,'554','Wheeling, WV-Steubenville, OH',-81.0436,39.9665,'24.6','93.2','69.8'),
	(199,'627','Wichita Falls, TX-Lawton, OK',-99.0398,33.9059,'48.8','90.7','42.5'),
	(200,'678','Wichita-Hutchinson, KS Plus',-99.0054,38.6729,'28.1','88.4','60.7'),
	(201,'577','Wilkes Barre-Scranton, PA',-76.5302,41.2488,'33.2','94.4','61.8'),
	(202,'550','Wilmington, NC',-78.2946,34.348,'30.2','92.1','62.4'),
	(203,'810','Yakima-Pasco-Richland-Kennewick, WA',-119.749,46.2958,'48.3','85.4','37.6'),
	(204,'536','Youngstown, OH',-80.5435,41.0506,'25.4','89','64'),
	(205,'771','Yuma, AZ-El Centro, CA',-114.72,32.7516,'43.8','81.3','38.3'),
	(206,'596','Zanesville, OH',-81.9657,39.9612,'24.1','94.3','70.3');

/*!40000 ALTER TABLE `geolocation_dma` ENABLE KEYS */;
UNLOCK TABLES;


# Dump of table geolocation_ip_locations
# ------------------------------------------------------------

DROP TABLE IF EXISTS `geolocation_ip_locations`;

CREATE TABLE `geolocation_ip_locations` (
  `id` int(30) unsigned NOT NULL AUTO_INCREMENT,
  `start_ip_num` int(30) NOT NULL,
  `end_ip_num` int(30) NOT NULL,
  `loc_id` int(30) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;



# Dump of table geolocation_weather_cache
# ------------------------------------------------------------

DROP TABLE IF EXISTS `geolocation_weather_cache`;

CREATE TABLE `geolocation_weather_cache` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `timestamp` int(20) NOT NULL,
  `postal_code` varchar(255) NOT NULL DEFAULT '',
  `city` varchar(255) NOT NULL DEFAULT '',
  `state_region` varchar(255) NOT NULL DEFAULT '',
  `lat` float NOT NULL,
  `lng` float NOT NULL,
  `nws_xml` text NOT NULL,
  `nws_page` varchar(255) NOT NULL DEFAULT '',
  `forecast_length` varchar(255) NOT NULL DEFAULT '',
  `data` longtext NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;




/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;
/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
