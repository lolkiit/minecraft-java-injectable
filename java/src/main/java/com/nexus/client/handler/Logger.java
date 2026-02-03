/*
	* ==================================== *
		* Project: Nexus Client (Java)
		* File: com/nexus/client/handler/Logger.java
		* Purpose: Console logging. info/success/warn/error with timestamp; logException for stack traces.
		* Author: lolkit
	* ==================================== *
*/

package com.nexus.client.handler;

import java.io.PrintWriter;
import java.io.StringWriter;
import java.text.SimpleDateFormat;
import java.util.Date;

public class Logger
{
    public static void info(String message) 
    {
        log("INFO", message);
    }

    public static void success(String message) 
    {
        log("SUCCESS", message);
    }

    public static void warn(String message) 
    {
        log("WARN", message);
    }

    public static void error(String message) 
    {
        log("ERROR", message);
    }

    public static void debug(String message) 
    {
        log("DEBUG", message);
    }

    private static void log(String level, String message) 
    {
        String logMessage = String.format("[%s] %s", level, message);
        System.out.println(logMessage);
    }

    public static void logException(String context, Throwable throwable) 
    {
        error(context + ": " + throwable.getMessage());
        StringWriter sw = new StringWriter();
        PrintWriter pw = new PrintWriter(sw);
        throwable.printStackTrace(pw);
        error(sw.toString());
    }
}
