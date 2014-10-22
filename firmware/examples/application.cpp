/**
 ******************************************************************************
 * @file    application.cpp
 * @authors  Satish Nair, Zachary Crockett and Mohit Bhoite
 * @version V1.0.0
 * @date    05-November-2013
 * @brief   Tinker application
 ******************************************************************************
  Copyright (c) 2013 Spark Labs, Inc.  All rights reserved.

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation, either
  version 3 of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this program; if not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/  
#include "application.h"
#include "aws-dynamodb-spark/AmazonDynamoDBClient.h"
#include "aws-dynamodb-spark/SparkAWSImplementations.h"
#include "aws-dynamodb-spark/AWSFoundationalTypes.h"
#include "aws-dynamodb-spark/keys.h"

int rssi = 0;

/* DYNAMODB SPECIFIC CONFIGURATION */

/* Contants describing DynamoDB table and values being used. */
static const char* HASH_KEY_NAME = "device";
static const char* HASH_KEY_VALUE = "Spark";
static const char* RANGE_KEY_NAME = "Time";
const char* TABLE_NAME = "sparkdb";
/* Constants for connecting to DynamoDB. */
const char* AWS_REGION = "us-east-1";
const char* AWS_ENDPOINT = "amazonaws.com";

/* 0 or 1 to determine state of the switch */
int switchState = 0;

/* Device independent implementations required for AmazonDynamoDBClient to
 * function. */
SparkHttpClient httpClient;
SparkDateTimeProvider dateTimeProvider;

AmazonDynamoDBClient ddbClient;

PutItemInput putItemInput;
ActionError actionError;

/* ***************************** */

SYSTEM_MODE(AUTOMATIC);

/* Get DeviceID */
String myIDStr = Spark.deviceID(); 
char * deviceId = new char[myIDStr.length() + 1];
strcpy(deviceId, myIDStr.c_str());

/* This function is called once at start up ----------------------------------*/
void setup()
{
	/* Begin serial communication. */
    	Serial.begin(9600);
    	/* Initialize ddbClient. */
    	ddbClient.setAWSRegion(AWS_REGION);
    	ddbClient.setAWSEndpoint(AWS_ENDPOINT);
    	ddbClient.setAWSSecretKey(awsSecKey);
    	ddbClient.setAWSKeyID(awsKeyID);
    	ddbClient.setHttpClient(&httpClient);
    	ddbClient.setDateTimeProvider(&dateTimeProvider);

	//My custom app here
	Spark.variable("rssi", &rssi, INT);

}

/* This function loops forever --------------------------------------------*/
void loop()
{
	//This will run in a loop
	rssi = WiFi.RSSI();
/* Create an Item. */
        AttributeValue deviceValue;
/*        deviceValue.setS(HASH_KEY_VALUE); */
        deviceValue.setS(deviceId); 
        AttributeValue timeValue;
        /* Getting current time for Time attribute. */
        timeValue.setS(dateTimeProvider.getDateTime());
        MinimalKeyValuePair < MinimalString, AttributeValue
                > att1(HASH_KEY_NAME, deviceValue);
        MinimalKeyValuePair < MinimalString, AttributeValue
                > att2(RANGE_KEY_NAME, timeValue);
        MinimalKeyValuePair<MinimalString, AttributeValue> itemArray[] = { att1,
                att2};

        /* Set values for putItemInput. */
        putItemInput.setItem(MinimalMap < AttributeValue > (itemArray, 2));
        putItemInput.setTableName(TABLE_NAME);

        /* perform putItem and check for errors. */
        PutItemOutput putItemOutput = ddbClient.putItem(putItemInput,
                actionError);
        switch (actionError) {
        case NONE_ACTIONERROR:
            Serial.println("PutItem succeeded!");
            Serial.println(myIDStr);
            Serial.println(rssi);
            break;
        case INVALID_REQUEST_ACTIONERROR:
            Serial.print("ERROR: ");
            Serial.println(putItemOutput.getErrorMessage().getCStr());
            break;
        case MISSING_REQUIRED_ARGS_ACTIONERROR:
            Serial.println(
                    "ERROR: Required arguments were not set for PutItemInput");
            break;
        case RESPONSE_PARSING_ACTIONERROR:
            Serial.println("ERROR: Problem parsing http response of PutItem");
            break;
        case CONNECTION_ACTIONERROR:
            Serial.println("ERROR: Connection problem");
            break;
        }
        /* wait to not double-record */
        delay(5000);


}
