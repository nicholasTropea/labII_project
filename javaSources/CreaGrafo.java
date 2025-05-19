// Librerie per operazioni IO
import java.io.BufferedReader;
import java.io.FileReader;
import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.IOException;

// Librerie per strutture dati
import java.util.Map;
import java.util.Date;
import java.util.Set;
import java.util.HashSet;
import java.util.HashMap;
import java.util.List;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;

// Librerie per logging
import java.util.logging.Level;
import java.util.logging.Logger;


/**
 * Costruisce un grafo di attori dai dataset IMDb.
 * Processa file TSV per identificare attori e le loro relazioni.
 */
public class CreaGrafo {
    private static Logger LOGGER;

    public static void main(String[] args) {        
        Level logLevel = Level.INFO; // Livello di default per debugging: INFO 
        
        if (!validateArguments(args)) System.exit(1);

        // Inizializza il logger
        LOGGER = CustomLogger.configureLogger(CreaGrafo.class, logLevel);
        Attore.setLogLevel(logLevel); // Imposta il logger della classe Attore
        LOGGER.info("Inizio esecuzione del programma con livello di log impostato a: " + logLevel);

        
        // Crea la map degli attori
        Map<Integer, Attore> attori = processActorsFile(args[0]);

        // Crea la map dei titoli
        Map<Integer, List<Integer>> titoli = processTitlesFile(args[1], attori);

        // Aggiorna il campo coprotagonisti di ogni attore
        updateCoprotagonisti(titoli, attori);

        // Converte la map degli attori in una lista e la ordina (non c'è bisogno di operare su di essa)
        // E' più efficiente usare tabelle hash rispetto a TreeSet/TreeMap ed ordinarle alla fine
        // piuttosto che avere costo O(log n) ad ogni operazione
        List<Attore> sortedAttori = new ArrayList<>(attori.values());
        sortedAttori.sort(Comparator.comparingInt(Attore::getCode));  // Java 8+ syntax

        // Lascia liberare memoria al Garbage Collector
        attori = null;
        titoli = null;

        // Crea il file nomi.txt
        createActorsFile(sortedAttori);

        // Crea il file grafo.txt
        createGraph(sortedAttori);

        LOGGER.info("Termine dell'esecuzione del programma.");
    }


    // ========== METODI PRIVATI ========== //

    /**
     * Valida gli argomenti da riga di comando.
     * @param args File name.basics.tsv e title.principals.tsv passati da riga di comando. 
     * @return True se gli argomenti passati sono validi, false atrimenti.
     */
    private static boolean validateArguments(String[] args) {
        if (args.length == 2) return true;
        
        LOGGER.severe("Numero di argomenti passati invalido, terminazione del programma.");
        System.out.println("Errore: Utilizzo incorretto.\nUso: java -Xmx8g CreaGrafo pathTo(name.basics.tsv) pathTo(title.principals.tsv)\n");
        return false;
    }


    /**
     * Processa il file degli attori e restituisce la mappa codici -> attori.
     * @param filename Il file name.basics.tsv 
     * @return Restituisce la TreeMap contenente gli attori validi.
     */
    private static Map<Integer, Attore> processActorsFile(String filename) {
        Map<Integer, Attore> attori = new HashMap<>();
        LOGGER.info(() -> "Inizio elaborazione file: " + filename);
        
        // Blocco try-with-resources, chiude automaticamente br all'uscita dal blocco.
        try (BufferedReader br = new BufferedReader(new FileReader(filename))) {
            br.readLine();  // Salta header
            
            String line;
            
            while ((line = br.readLine()) != null) {
                String[] fields = parseTSV(line);

                // Se non è un attore valido lo salta
                if (fields == null || "\\N".equals(fields[2]) || !containsActorRole(fields[4])) continue;

                // Crea nodo attore
                Attore node = new Attore(fields[1], fields[0], fields[2]);
                attori.put(node.getCode(), node);
            }                
        }
        catch (IOException e) {
            LOGGER.log(Level.SEVERE, "Errore nella lettura del file " + filename, e);
            System.exit(2);
        }
        
        return attori;
    }


    /**
     * Parsa una riga TSV.
     * @param line Linea del file estratta in processActorsFile() o processTitlesFile().
     * @return Restituisce l'array con i campi del file se line è valida, null altrimenti.
     * @throws IllegalArgumentException Se trova una linea con più o meno campi del previsto.
     */
    private static String[] parseTSV(String line) {
        // Restituisce null se line è vuota
        if (line == null || line.trim().isEmpty()) {
            if (LOGGER.isLoggable(Level.FINEST)) LOGGER.finest("Linea TSV vuota o nulla.");
            return null;
        }

        // Fatto in questo modo per motivi di efficienza (meno riallocazioni, meno memoria occupata)
        List<String> fields = new ArrayList<>(6); // Capacità iniziale
        int start = 0;
        int end = line.indexOf('\t');
        
        while (end != -1) {
            fields.add(line.substring(start, end));
            start = end + 1;
            end = line.indexOf('\t', start);
        }

        fields.add(line.substring(start)); // Aggiungi l'ultimo campo

        // Lancia un'eccezione se line è invalida
        if (fields.size() != 6) {
            LOGGER.severe(() -> "Line invalida -> Campi insufficienti: " + fields.size());
            throw new IllegalArgumentException("Formato TSV non valido.");
        }

        return fields.toArray(new String[0]);
    }


    /**
     * Verifica se tra i ruoli è presente 'actor' o 'actress'.
     * @param roles Campo primaryProfession del file name.basics.tsv passato da parseTSV().
     * @return Restituisce true se il campo contiene actor/actress, false altrimenti.
     */
    private static boolean containsActorRole(String roles) {
        for (String role : roles.split(",")) {
            if (role.trim().equalsIgnoreCase("actor") || 
                role.trim().equalsIgnoreCase("actress")) {
                return true;
            }
        }
        return false;
    }


    /** 
     * Crea il file nomi.txt in formato tsv: codice nome dataDiNascita.
     * @param attori HashMap creata in processActorsFile().
     */
    private static void createActorsFile(List<Attore> attori) {
        try (BufferedWriter writer = new BufferedWriter(new FileWriter("nomi.txt"))) {
            LOGGER.info("Inizio scrittura su nomi.txt");            

            for (Attore a : attori) {
                String line = String.format("%d\t%s\t%s", a.getCode(), a.getName(), a.getDate());
                writer.write(line);
                writer.newLine();
            }

            LOGGER.info("Fine scrittura su nomi.txt");
        }
        catch (IOException e) {
            LOGGER.log(Level.SEVERE, "Errore nella scrittura del file nomi.txt", e);
        }
    }


    /**
     * Processa il file dei titoli e restituisce la mappa codici -> titoli.
     * @param filename Il file title.principals.tsv
     * @param attori TreeMap contenente i nodi degli attori costruita in processActorsFile().
     * @return Restituisce un HashSet contenente i nodi dei titoli.
     */
    private static Map<Integer, List<Integer>> processTitlesFile(String filename, Map<Integer, Attore> attori) {
        Map<Integer, List<Integer>> titoli = new HashMap<>();
        
        // Blocco try-with-resources, chiude automaticamente br all'uscita dal blocco
        try (BufferedReader br = new BufferedReader(new FileReader(filename))) {
            LOGGER.info(() -> "Inizio elaborazione file: " + filename);

            br.readLine(); // Salta header

            String line;

            while ((line = br.readLine()) != null) {
                String[] fields = parseTSV(line);

                // In caso di linea vuota salta
                if (fields == null) continue;

                int titleCode = parseTitleCode(fields[0]);
                int actorCode = Attore.getActorCode(fields[2]);

                // Se attore o titolo non valido salta
                if (titleCode == -1 || actorCode == -1 || !attori.containsKey(actorCode)) continue;

                if (!titoli.containsKey(titleCode)) titoli.put(titleCode, new ArrayList<Integer>());

                // Non ci sono duplicati, ma anche se ci fossero non sarebbe un problema
                // perché coprotagonisti è un set, quindi saranno unici.
                titoli.get(titleCode).add(actorCode);
            }

            LOGGER.info("Termine elaborazione file: " + filename);
        }
        catch (IOException e) {
            LOGGER.log(Level.SEVERE, "Errore nella lettura del file " + filename, e);
            System.exit(2);
        }

        return titoli;
    }


    /**
     * Calcola il codice univoco di un titolo.
     * @param codice Il codice del titolo in formato "tt1234567".
     * @return Il codice univoco del titolo senza il prefisso "tt", -1 se il codice non è valido.
     */
    private static int parseTitleCode(String codice) {
        if (codice == null || codice.length() <= 2) return -1;

        try {
            return Integer.parseInt(codice.substring(2));
        }
        catch (NumberFormatException e) {
            return -1;
        }
    }


    /**
     * Aggiunge al campo coprotagonisti di ogni attore quelli con cui ha recitato.
     * @param titoli HashMap contenente i titoli e i relativi cast creata in processTitlesFile().
     * @param attori TreeMap contenente i nodi Attore creata in processActorsFile().
     */
    private static void updateCoprotagonisti(Map<Integer, List<Integer>> titoli, Map<Integer, Attore> attori) {
        LOGGER.info("Inizio aggiornamento dei coprotagonisti.");

        // Itera i cast
        for (List<Integer> cast : titoli.values()) {
            for (int actorCode : cast) {
                Attore current = attori.get(actorCode);

                // Check per sicurezza (non necessario), aggiunge tutti gli attori (rimuove se stesso quando crea il grafo)
                if (current != null) current.getCoprotagonisti().addAll(cast);
            }
        }

        LOGGER.info("Termine aggiornamento dei coprotagonisti.");
    }

    
    /**
     * Crea il file grafo.txt
     * @param attori TreeMap dei nodi Attore costruita in processActorsFile().
     */
    private static void createGraph(List<Attore> attori) {
        try (BufferedWriter writer = new BufferedWriter(new FileWriter("grafo.txt"))) {
            LOGGER.info("Inizio scrittura su grafo.txt");

            for (Attore a : attori) {
                // Rimuove se stesso dai coprotagonisti
                a.getCoprotagonisti().remove(a.getCode());

                // Array con size iniziale 500, converto in array per sort più efficiente
                List<Integer> sortedCoprot = new ArrayList<>(500);
                sortedCoprot.addAll(a.getCoprotagonisti()); // Riempie l'array
                Collections.sort(sortedCoprot); // Sort più efficiente per ~ 1k elementi (TimSort)
                
                // Utilizzato perché è il metodo più efficiente per concatenazione in loop
                StringBuilder line = new StringBuilder();
                line.append(a.getCode()).append("\t").append(sortedCoprot.size());

                for (int code : sortedCoprot) {
                    line.append("\t").append(code);
                }

                writer.write(line.toString());
                writer.newLine();
            }

            LOGGER.info("Fine scrittura su grafo.txt");
        }
        catch (IOException e) {
            LOGGER.log(Level.SEVERE, "Errore nella scrittura del file grafo.txt", e);
        }
    }
}