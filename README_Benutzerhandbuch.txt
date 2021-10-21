==========================
BENUTZERHANDBUCH
==========================

Vorwort:

Dieses Programm multipliziert zwei dünnbesetzte Matrizen A und B miteinander. 

Sie haben entweder die Möglichkeit, die zu multiplizierenden Matrizen selber via zwei Textdateien 
"inputA.txt" und "inputB.txt" einzugeben. Beachten Sie hierzu die im Abschnitt "File I/O-Format" 
beschriebene Syntax. Nachdem die Multiplikation erfolgt ist, finden Sie das Ergebnis, sofern eine
Multiplikation möglich war, im selben Format in der Textdatei "result.txt".

Eine andere Möglichkeit ist es vorgefertigte Matrizen miteinander zu multiplizieren. Hierfür ist
keine Eingabe möglich, allerdings sollten Sie sich dennoch mit dem Ein-/Ausgabeformat vertraut
machen. Alle vorgefertigten Tests werden gleichzeitig ausgeführt. Die Ergebnisse finden Sie in den
Unterordnern des "tests"-Ordner in den jeweiligen "result.txt"-Dateien. Um die Korrektheit des
Ergebnisses zu überprüfen, ist jedem Test eine "expectedResult.txt"-Datei beigelegt, die das
erwartete Ergebnis liefert.

--------------------------

Erzeugen von nquad-Matrizen zu Testzwecken:

nquad-Matrizen sind quadratische Matrizen der Dimension (2^n -1)^2 mit n von Null verschiedenen 
Werten pro Zeile. Sie können nützlich sein, um die Implementierung zu testen.
Mit quadgen.c können Textdateien mit nquad-Matrizen erzeugt werden. Diese sind dann in ./generated/
zu finden.

	Nutzung:

	gcc quadgen.c -o quadgen && ./quadgen n datei.txt
          
        mit n einer ganzen Zahl in [0,64] und "datei.txt" der gewünschten Ausgabedatei.

	WICHTIG: Bitte geben Sie die Ausgabedatei mit ".txt" am Ende an.

--------------------------

File I/O-Format:

Die erste Zeile einer eingebenen Matrix beschreibt ihre grundlegenden Eigenschaften:
Der erste Wert entspricht der Anzahl an Zeilen der eingebenen Matrix, der zweite seiner Anzahl an
Spalten. Der dritte Wert der ersten Zeile ist die Anzahl an Werten ungleich 0, die die Matrix
enthält.
WICHTIG: - Die Anzahl der erwarteten Zeilen nach dieser ersten Zeile entspricht der Anzahl an
           angegebenen Werten ungleich 0. Geben Sie also beispielsweise 5 Werte ungleich 0 an, so
           erwartet das Programm nach der ersten Zeile noch 5 weitere, also insgesamt 6 Zeilen
           inklusive der ersten.
         - Trennen Sie die drei Werte jeweils durch ein Leerzeichen. Eine Trennung durch mehr als
           nur ein Leerzeichen führt zu einer Fehlermeldung. Dies gilt auch für jede darauffolgende
           Zeile.
         - Sollte die Anzahl an Werten ungleich 0 UINT64_MAX = 18446744073709551615 übersteigen, so
           soll sie als arithmetische Operation aus Werten <= UINT64_MAX eingegeben werden.

Jede weitere Zeile entspricht einem drei-Tupel aus einem eingebenen Wert (kann auch eine
Dezimalzahl sein), seinem Zeilenindex und seinem Spaltenindex.
WICHTIG: - Die drei-Tupel müssen nach der folgenden Regel sortiert eingegeben werden: Die Werte
           werden vom kleinsten zum größten Zeilenindex eingegeben, und für jeden Zeilenindex vom
           kleinsten zum größten Spaltenindex. Zum Beispiel muss ein Wert in Zeile 2, Spalte 5 vor
           einem Wert in Zeile 3, Spalte 0 eingegeben werden, und dieser wiederum vor einem Wert in
           Zeile 3, Spalte 1. Eine Missachtung dieser Sortierung führt zu einer Fehlermeldung. 
         - Der Zeilen- und der Spaltenindex müssen ganze Zahlen sein. Dezimalzahlen sind nicht 
           gestattet und führen zur Ausgabe einer Fehlermeldung.
         - Indizes beginnen bei 0: Hat also eine Matrix zum Beispiel 5 Zeilen, so gehen die Zeilen-
           indizes von 0 bis 4.

Da das Programm zur Multiplikation von dünnbesetzten Matrizen benutzt werden soll, ist es
wahrscheinlich, dass die Dimension der eingegebenen Matrizen deren Anzahl an Werten ungleich 0
übersteigt. Alle weiteren nicht explizit eingebenen Werte werden als 0 interpretiert.

Die Eingabe von Dezimalzahlen wird unterstützt (natürlich nur für die Werte und nicht den Zeilen-
und Spaltenindizes). Trennen Sie dafür Vor- und Nachkommastellen durch einen Punkt. Sollten Sie
stattdessen ein Komma verwenden, so wird eine Fehlermeldung ausgegeben.
Eine Eingabe der Form 0.X kann auch direkt in der Form .X geschrieben werden: Beispielsweise wird
die Eingabe .5 als 0.5 interpretiert.

Beispiel: (0 1  2  )          5 3 6
          (0 3  0  )          1 0 1
          (0 0 5,35)    ->    2 0 2
          (4 0  0  )          3 1 1
          (0 6  0  )          5.35 2 2
                              4 3 0
                              6 4 1

WICHTIG: Fügen Sie keine Leerzeilen, sowohl zwischen den Einträgen als auch am Ende der Dateien, 
         ein, da sonst eine Fehlermeldung ausgegeben wird.

--------------------------

Nutzung:

WICHTIG: - Es wird die Matrix in "inputA.txt" mit der Matrix in "inputB.txt" multipliziert, das 
           heißt "result.txt" entspricht dem Ergebnis von A * B. Beachten Sie deshalb, das oft 
           A * B und B * A ein unterschiedliches Ergebnis liefern.
         - Sollte die Multiplikation zweier Matrizen nicht möglich sein, da ihre Dimensionen nicht
           kompatibel sind, so wird eine Fehlermeldung auf der Kommandozeile ausgegeben.

	Standardmodus:

        ./main [-2] [-u] [-b]: Es werden die sich im Ordner "Implementierung" befindenden 
                               "inputA.txt" und "inputB.txt" Dateien (sofern möglich) miteinander
                               multipliziert.
                               Standardmäßig wird der optimierte Algorithmus mit den Überprüfungen 
                               matr_mult_csr_SIMD (Algorithm 1) ausgeführt. Wird die Ausführung des 
                               Algorithmus mit der Transponierung matr_mult_csr2 (Algorithm 2) 
                               gewünscht, so muss zusätzlich die Option "-2" auf der Kommandozeile 
                               angegeben werden. Merke: Es gibt keine optimierte Version des
			       zweiten Algorithmus'.
                               Die Option "-u" führt die unoptimierten Implementierungen 
                               matr_mult_csr respektive matr_mult_csr2 aus.
                               Die Option "-b" gibt die Zeitmessergebnisse auf der Konsole aus.

	Testmodus:

        ./main -t [-2] [-u] [-b]: Im Ordner "Implementierung/tests" befinden sich eine Vielzahl an 
                                  Unterordnern. In jedem Unterordner befinden sich bereits 
                                  ausgefüllte "inputA.txt" und "inputB.txt" Matrizen, die jeweils 
                                  miteinander multipliziert werden. Das Ergebnis der Multiplikation
                                  finden Sie ebenfalls in den Unterordnern in der Datei 
                                  "result.txt". Zudem befindet sich in jedem Unterordner als 
                                  Vergleich bereits vor Ausführung der Tests das erwartete Ergebnis
                                  in "expectedResult.txt".
                                  Standardmäßig wird der optimierte Algorithmus mit den 
                                  Überprüfungen matr_mult_csr_SIMD (Algorithm 1) ausgeführt. Wird 
                                  die Ausführung des Algorithmus mit der Transponierung 
                                  matr_mult_csr2 (Algorithm 2) gewünscht, so muss zusätzlich die
                                  Option "-2" auf der Kommandozeile angegeben werden. Merke: Es 
                                  gibt keine optimierte Version des zweiten Algorithmus'.
                                  Die Option "-u" führt die unoptimierten Implementierungen 
                                  matr_mult_csr respektive matr_mult_csr2 aus.
                                  Die Option "-b" gibt die Zeitmessergebnisse auf der Konsole aus.

        ./main -t -q [-2] [-u] [-b]: Anstelle der Tests in "Implementierungs/tests" werden die 
                                     quadratischen Matrizen in den Unterordnern von 
                                     "Implementierung/nquad" miteinander multipliziert. Dies ist vor
                                     allem zu Benchmarking Zwecken vorgesehen.

        WICHTIG: Sollte die Option "-q" verwendet werden, so muss vorher die "nquad.zip" in
                 "Implementierung" in den selben Ordner entpackt werden.

./main -h: Gibt Nutzungshinweise auf der Kommandozeile aus.