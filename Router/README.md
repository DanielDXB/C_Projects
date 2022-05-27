# Router

### Daniel Bolontoc &copy; 2022

Protocoale de comunicare
<li>
Procesul de dirijare <br>
<li>
Longest prefix match eficient <br>
<li>
Protocolul ICMP <br>
<br>
	
	Pentru procesul de dirijar, initial parsez si sortez o tabela de rutare.
	Sortarea tebelei este realizate cu functia qsort criteriul principal fiind valoarea prefixului, urmata de valoarea mastii.
	
	Verific daca pachetul este pentru mine si daca este de tip ipv4 il prelucrez.
	Functia ip_packet este cea care prelucreaza pachetele ipv4 si intoarce diferite coduri de eroare in functie de starea acestuia.
	
<li>	
 0 = checksum gresit, pachetul este abandonat <br>
<li>
-1 = ttl este mai mic sau egal cu 1, se trimite un icmp pentru time exceeded <br>
<li>
-2 = pachetul este defapt un icmp de tim echo request, se trimite un echo reply <br>
<li>
-3 = tabela de rutare nu gaseste urmatorul router, se trimite un icmp de tip destination unreachable
<br>
	<br>

	Pentru a creea un pachet icmp se actualizeaza headerul de ip si in urma sa creez headerul de icmp.

	LPM este implementat eficient, folosind o cautare binara.


	

