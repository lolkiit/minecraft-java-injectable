/*
	* ==================================== *
		* Project: Nexus Client (Java)
		* File: com/nexus/client/injection/EntryPoint.java
		* Purpose: Injection entry point. Called by mapper via JNI; starts client and registers shutdown hook.
		* Author: lolkit
	* ==================================== *
*/

package com.nexus.client.injection;

import com.nexus.client.BaseClient;
import com.nexus.client.handler.Logger;

public class EntryPoint 
{
    public static void init() 
    {
        Logger.info("Starting Nexus Client initialization...");

        try {
            BaseClient.initialize();

            Runtime.getRuntime().addShutdownHook(new Thread(() -> {
                Logger.info("Shutdown hook triggered");
                BaseClient.shutdown();
            }));

        } catch (Throwable t) {
            Logger.logException("Critical error in entry point", t);
        }
    }

    public static void main(String[] args) 
    {
        Logger.info("Running in standalone mode (testing)");
        init();
    }
}
