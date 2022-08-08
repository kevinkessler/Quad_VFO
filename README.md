Si5351 VFO Quadrature signal generation 
=======================================

This project demonstrates creating Quadrature signals using the Si5351. It uses my fork of the Etherkit/Si5351Arduino library which allows 
generation of quadrature signals through the set_phase function down through the 80M bands, as discussed by Charlie Morris here: [https://www.youtube.com/watch?v=XO6TSkcpwOk&ab_channel=CharlieMorris](https://www.youtube.com/watch?v=XO6TSkcpwOk&ab_channel=CharlieMorris). 
For frequencies below 3.2Mhz, it uses the delay method of creating the quadrature signals, as described in this document: 
[https://tj-lab.org/2020/08/27/si5351%e5%8d%98%e4%bd%93%e3%81%a73mhz%e4%bb%a5%e4%b8%8b%e3%81%ae%e7%9b%b4%e4%ba%a4%e4%bf%a1%e5%8f%b7%e3%82%92%e5%87%ba%e5%8a%9b%e3%81%99%e3%82%8b/](https://tj-lab.org/2020/08/27/si5351%e5%8d%98%e4%bd%93%e3%81%a73mhz%e4%bb%a5%e4%b8%8b%e3%81%ae%e7%9b%b4%e4%ba%a4%e4%bf%a1%e5%8f%b7%e3%82%92%e5%87%ba%e5%8a%9b%e3%81%99%e3%82%8b/) 
(in Japanese, google translate works well). 

Hans Summers uses this method in his QDX transceivers for the 160M band, which is discussed here: [https://groups.io/g/QRPLabs/topic/qdx_on_630m/91086918?p=,,,20,0,0,0::recentpostdate/sticky,,,20,2,20,91086918,previd%3D1653402103835960557,nextid%3D1653251209276070476&previd=1653402103835960557&nextid=1653251209276070476](https://groups.io/g/QRPLabs/topic/qdx_on_630m/91086918?p=,,,20,0,0,0::recentpostdate/sticky,,,20,2,20,91086918,previd%3D1653402103835960557,nextid%3D1653251209276070476&previd=1653402103835960557&nextid=1653251209276070476)

