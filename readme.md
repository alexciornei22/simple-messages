## CIORNEI ALEXANDRU-ȘTEFAN, 324CD

## TEMA 2 PCOM

---

### Fișiere:

- *Socket.cpp*: Clasa wrapper peste functiile de socket din C, implementează
protocolul de trimitere a mesajelor. Se folosește funcția ``recvNBytes()``
pentru a primi calupuri de n octeti de pe socket, aceasta fiind folosită
de functia ``recvvMessage()`` pentru a primi mai intai header-ul unui mesaj,
urmat de datele incluse in acesta, care pot avea o lungime variabila, in
funcția de lungimea citită în header.

- *TCPServer.cpp*/*TCPClient.cpp*: Clase care implementează funcționalitățile
server-ului și ale clientului TCP. Se folosesc de metodele socket-ului pentru
a comunica.

- *server.cpp*/*subscriber.cpp*: Codul "driver" pentru server/subscriber

- *protocol.hpp*: Macro-uri si structuri care definesc protocolul și ajută
la implementare

### Funcționare
Server-ul isi initializeaza structurile interne, începând prin a deschide un socket
TCP pentru a primi conexiuni și prin a inițializa un vector de structuri ``pollfd``
pentru a multiplexa I/O-ul, în 4 cazuri (input de la ``stdin``, input de la o nouă
conexiune, input de la socket-ul UDP, input de la un client TCP conectat). Server-ul
memorează câte un array pentru Topic-uri și Clienți. Clienții sunt stocați odată ce
se primește mesajul de conectare, iar Topic-urile odată ce se primește un mesaj UDP
pe acel topic.

Clasele ``Topic`` și ``Client`` interacționează după **Design Pattern-ul Observer**,
prin metodele ``notifyClients()`` și ``update()``, făcând mai intuitivă implementarea
celor 2 entități.