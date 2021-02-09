window.poner_svg=function (padre=document.getElementsByClassName('tsinbo bsbb')[0]){
    //var tablero=document.getElementsByClassName('tsinbo bsbb')[0]
    svg=document.createElementNS('http://www.w3.org/2000/svg','svg')
    //svg.setAttribute('style','position: absolute; width: 100%; height: 100%; text-align: center; z-index: 70;')
    svg.setAttribute('width',"100%")    
    svg.setAttribute('height',"100%")
    //tablero.appendChild(svg)
    //padre.appendChild(svg)
	padre.insertBefore(svg,padre.firstElementChild)
}

window.quitar_svg=function(){
	var svg=document.getElementsByTagName('svg')[0]
	svg.parentNode.removeChild(svg)
}

window.rectangulo=function (x,y,ancho,alto){
    rect=document.createElementNS('http://www.w3.org/2000/svg','rect')
    rect.setAttribute('width', ancho);
    rect.setAttribute('height', alto);
    rect.setAttribute('y', y);
    rect.setAttribute('x', x);
    rect.setAttribute('style', "stroke-width:1;stroke:blue;fill;none");
    //stroke es el borde (y se iguala al color de este)
    //stroke width es la anchura del borde
    //fill es el relleno
    rect.style.visibility='visible'

    var svg=document.getElementsByTagName('svg')[0]
    svg.appendChild(rect)
}


window.cuadrado=function (x,y,l){
    rect=document.createElementNS('http://www.w3.org/2000/svg','rect')
    rect.setAttribute('width', l);
    rect.setAttribute('height', l);
    rect.setAttribute('y', y);
    rect.setAttribute('x', x);
    rect.setAttribute('style', "stroke-width:1;stroke:blue;fill:none");
    //stroke es el borde (y se iguala al color de este)
    //stroke width es la anchura del borde
    //fill es el relleno
    rect.style.visibility='visible'

    return rect
}

//x,y son las coordenadas ("offset") de la esquina superior izquierda del tablero
window.encuadrar=function (x,y,longitud){
    var svg=document.getElementsByTagName('svg')[0]

    l=longitud/8
    var i;
    var j;

    for(i=0;i<8;i++){
        for(j=0;j<8;j++){
            svg.appendChild(cuadrado(x+i*l,y+j*l,l))
        }
    }

    
}
//encuadrar(20,59,483)




window.circulo=function (r,x,y){
    // NO FUNCINONA
    circ=document.createElementNS('http://www.w3.org/2000/svg','circle')
    circ.setAttribute('r',r)
    circ.setAttribute('cx',x)
    circ.setAttribute('cy',y)
    //circ.setAttribute('stroke','red')
    //circ.setAttribute('stroke-width',5)
    //circ.setAttribute('fill',null)
    circ.setAttribute('style', "stroke-width:1;stroke:blue;fill:none");
    circ.style.visibility='visible'

    var svg=document.getElementsByTagName('svg')[0]
    svg.appendChild(circ)
}


window.linea=function (x1=250,y1=400,x2=250,y2=300){
    line=document.createElementNS('http://www.w3.org/2000/svg','line')
    line.setAttribute('stroke','blue')
    //line.setAttribute('stroke',"#15781B")
    line.setAttribute('stroke-width','9.375')
    line.setAttribute('stroke-linecap','round')
    //line.setAttribute('marker-end',"url(#mipunta)")
    //line.setAttribute('marker-end',null)
    //line.setAttribute('marker-end',"url(#arrowhead-g)")
    line.setAttribute('opacity','1')
    line.setAttribute('x1',x1)
    line.setAttribute('y1',y1)
    line.setAttribute('x2',x2)
    line.setAttribute('y2',y2)
    line.setAttribute('cgHash',"480,480,g2,g4,green")
    line.style.visibility='visible'

    var svg=document.getElementsByTagName('svg')[0]
    svg.appendChild(line)
    
}

//poner_svg()
