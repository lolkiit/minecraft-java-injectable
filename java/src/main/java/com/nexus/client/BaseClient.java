/*
	* ==================================== *
		* Project: Nexus Client (Java)
		* File: com/nexus/client/BaseClient.java
		* Purpose: Main client class. Lifecycle (initialize/shutdown), version info, Minecraft hook.
		* Author: lolkit
	* ==================================== *
*/

package com.nexus.client;

import com.nexus.client.handler.Logger;

public class BaseClient 
{
    private static final String CLIENT_NAME = "Nexus Client";
    private static final String CLIENT_VERSION = "1.0.0";
    private static final String MC_VERSION = "1.8.9";

    private static boolean initialized = false;

    public static void initialize()
    {
        if (initialized)
        {
            Logger.warn("Client already initialized!");
            return;
        }

        try {
            Logger.info("========================================");
            Logger.info(" " + CLIENT_NAME + " v" + CLIENT_VERSION);
            Logger.info(" Minecraft Version: " + MC_VERSION);
            Logger.info("========================================");

            Logger.success("Client initialized successfully!");
            initialized = true;

        } catch (Exception e) {
            Logger.error("Failed to initialize client: " + e.getMessage());
            e.printStackTrace();
        }
    }

    public static void shutdown() 
    {
        if (!initialized) 
        {
            return;
        }

        Logger.info("Shutting down " + CLIENT_NAME + "...");

        try {
            Logger.success("Client shutdown successfully!");
        } catch (Exception e) {
            Logger.error("Error during shutdown: " + e.getMessage());
        }

        initialized = false;
    }

    public static boolean isInitialized() 
    {
        return initialized;
    }
}