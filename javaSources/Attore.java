// Librerie per strutture dati
import java.util.Set;
import java.util.HashSet;

// Librerie per logging
import java.util.logging.Level;
import java.util.logging.Logger;


/** Rappresenta un nodo attore. */
public class Attore {
    private static Logger LOGGER;

    /**
     * Imposta il livello del logger, viene chiamata all'inizio del main di CreaGrafo.java
     * @param level Livello impostato dalla classe CreaGrafo.
     */
    public static void setLogLevel(Level level) {
        LOGGER = CustomLogger.configureLogger(Attore.class, level);
    }

    /** Nome dell'attore. */
    private String nome;

    /** Codice univoco dell'attore. */
    private int codice;

    /** Anno di nascita dell'attore. */
    private int anno;

    /** HashSet contenente i codici degli attori con cui this ha recitato. */
    private Set<Integer> coprotagonisti;
    
    /**
     * Costruttore della classe Attore.
     * 
     * @param nome Il nome dell'attore.
     * @param codice Il codice dell'attore in formato "nm1234567".
     * @param anno L'anno di nascita dell'attore.
     * @throws IllegalArgumentException Se il codice dell'attore è una stringa non valida.
     * @throws NumberFormatException Se il codice (senza nm) o l'anno di nascita dell'attore non sono numeri interi.
     */
    public Attore(String nome, String codice, String anno) throws IllegalArgumentException, NumberFormatException {
        this.nome = nome;

        this.codice = getActorCode(codice);
        if (this.codice == -1) {
            throw new IllegalArgumentException("Codice attore non valido: " + codice);
        }

        try {
            this.anno = Integer.parseInt(anno);
        }
        catch (NumberFormatException e) {
            LOGGER.severe("Data di nascita dell'attore non valida, terminazione del programma.");
            throw new NumberFormatException("Anno di nascita dell'attore non valido: " + anno);
        }

        this.coprotagonisti = new HashSet<Integer>();
    }

    /**
     * Calcola il codice univoco di un attore.
     * @param codice il codice dell'attore in formato "nm1234567".
     * @return il codice univoco dell'attore senza il prefisso "nm", -1 se il codice non è valido
     */
    public static int getActorCode(String codice) {        
        if (codice == null || codice.length() <= 2) {
            LOGGER.severe("Codice attore invalido, terminazione del programma.");
            return -1;
        }
        
        try {
            return Integer.parseInt(codice.substring(2));
        }
        catch (NumberFormatException e) {
            LOGGER.severe("Codice attore invalido, terminazione del programma.");
            return -1;
        }
    }

    /**
     * Getter del codice identificativo dell'attore.
     * @return Codice identificativo dell'attore.
     */
    public int getCode() {
        return this.codice;
    }
    
    /**
     * Getter del nome dell'attore.
     * @return Nome dell'attore.
     */
    public String getName() {
        return this.nome;
    }

    /**
     * Getter della data di nascita dell'attore.
     * @return Data di nascita dell'attore.
     */
    public int getDate() {
        return this.anno;
    }

    /**
     * Getter del set dei coprotagonisti dell'attore.
     * @return HashSet contenente i coprotagonisti dell'attore.
     */
    public Set<Integer> getCoprotagonisti() {
        return this.coprotagonisti;
    }
}