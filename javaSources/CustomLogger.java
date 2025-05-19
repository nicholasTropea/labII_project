// Libreria per il logging
// Livelli: (FINEST -> FINER -> FINE -> CONFIG -> INFO -> WARNING -> SEVERE -> OFF)
import java.util.logging.*;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.io.IOException;

public class CustomLogger {
    private static FileHandler sharedFileHandler; // Unico handler condiviso
    
    /**
     * Configura un logger per la classe passata.
     * @param clazz La classe per cui configurare il logger.
     * @param level Livello minimo dei messaggi loggati.
     * @return Istanza del logger configurato.
     */
    public static Logger configureLogger(Class<?> clazz, Level level) {
        Logger logger = Logger.getLogger(clazz.getName());
        logger.setUseParentHandlers(false);
        logger.setLevel(level);

        try {
            if (sharedFileHandler == null) {
                // Crea l'handler solo una volta
                sharedFileHandler = new FileHandler("CreaGrafo.log", true); // append=true
                sharedFileHandler.setLevel(level);

                sharedFileHandler.setFormatter(new SimpleFormatter() {
                    private final SimpleDateFormat sdf = 
                        new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");

                    @Override
                    public String format(LogRecord record) {
                        return String.format(
                            "%s [%s] - %s - %s%n",
                            sdf.format(new Date(record.getMillis())),
                            record.getLevel(),
                            record.getLoggerName(),
                            record.getMessage()
                        );
                    }
                });
            }

            logger.addHandler(sharedFileHandler); // Riutilizza l'handler esistente
        }
        catch (IOException e) {
            System.err.println("Errore nel setup del logger: " + e);
        }
        
        return logger;
    }
}