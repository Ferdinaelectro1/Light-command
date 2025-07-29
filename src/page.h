#pragma once
#include <Arduino.h>
const char code[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="fr">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>YOUPILIGHT</title>
    <style>
        /* Styles généraux */
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            line-height: 1.6;
            margin: 0;
            padding: 0;
            background-color: #f4f7f6;
            color: #333;
            display: flex;
            flex-direction: column;
            min-height: 100vh;
        }

        /* En-tête */
        header {
            background-color: #2c3e50;
            color: #fff;
            padding: 1rem 0;
            text-align: center;
            box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
        }

        header h1 {
            margin: 0;
            font-size: 2.5em;
            letter-spacing: 2px;
        }

        header nav ul {
            padding: 0;
            list-style: none;
            margin-top: 10px;
        }

        header nav ul li {
            display: inline;
            margin: 0 15px;
        }

        header nav ul li a {
            color: #fff;
            text-decoration: none;
            font-weight: bold;
            transition: color 0.3s ease;
        }

        header nav ul li a:hover {
            color: #1abc9c;
        }

        /* Contenu principal */
        main {
            flex: 1;
            max-width: 900px;
            margin: 2rem auto;
            padding: 20px;
            background-color: #ffffff;
            border-radius: 8px;
            box-shadow: 0 4px 10px rgba(0, 0, 0, 0.05);
        }

        section {
            margin-bottom: 2.5rem;
            padding: 1.5rem;
            border: 1px solid #e0e0e0;
            border-radius: 6px;
            background-color: #fcfcfc;
        }

        section h2 {
            color: #2c3e50;
            font-size: 1.8em;
            margin-top: 0;
            margin-bottom: 1.5rem;
            border-bottom: 2px solid #1abc9c;
            padding-bottom: 10px;
        }

        /* Section Heure actuelle */
        section:first-of-type p {
            font-size: 3em;
            text-align: center;
            font-weight: bold;
            color: #1abc9c;
            letter-spacing: 3px;
        }

        /* Formulaires et champs */
        form#config-form fieldset {
            border: 1px solid #d0d0d0;
            padding: 1.5rem;
            margin-bottom: 1.5rem;
            border-radius: 6px;
            background-color: #fff;
        }

        form#config-form legend {
            font-weight: bold;
            color: #2c3e50;
            font-size: 1.2em;
            padding: 0 10px;
            background-color: #fcfcfc;
            border-radius: 4px;
        }

        form#config-form label {
            display: inline-block;
            width: 80px;
            margin-bottom: 0.8rem;
            color: #555;
            font-weight: 500;
        }

        form#config-form input[type="number"] {
            width: calc(100% - 100px); /* Ajuster la largeur en fonction du label */
            padding: 10px;
            margin-bottom: 0.8rem;
            border: 1px solid #ccc;
            border-radius: 4px;
            box-sizing: border-box;
            transition: border-color 0.3s ease;
        }

        form#config-form input[type="number"]:focus {
            border-color: #1abc9c;
            outline: none;
            box-shadow: 0 0 5px rgba(26, 188, 156, 0.3);
        }

        /* Boutons */
        button {
            background-color: #1abc9c;
            color: white;
            padding: 12px 25px;
            border: none;
            border-radius: 5px;
            cursor: pointer;
            font-size: 1em;
            transition: background-color 0.3s ease, transform 0.2s ease;
            margin-right: 10px;
        }

        button:hover {
            background-color: #16a085;
            transform: translateY(-2px);
        }

        button:active {
            transform: translateY(0);
        }

        #btn_reglage {
            display: block;
            width: 100%;
            margin-top: 2rem;
            background-color: #2c3e50;
        }

        #btn_reglage:hover {
            background-color: #34495e;
        }

        #on_lampe {
            background-color: #28a745; /* Vert pour Allumer */
        }

        #on_lampe:hover {
            background-color: #218838;
        }

        #off_lampe {
            background-color: #dc3545; /* Rouge pour Éteindre */
        }

        #off_lampe:hover {
            background-color: #c82333;
        }

        /* État de la lampe */
        #etat_lampe {
            font-size: 1.2em;
            margin-right: 20px;
            font-weight: bold;
            color: #2c3e50;
        }

        /* Pied de page */
        footer {
            background-color: #2c3e50;
            color: #fff;
            text-align: center;
            padding: 1rem 0;
            margin-top: 2rem;
            box-shadow: 0 -2px 4px rgba(0, 0, 0, 0.1);
        }

        footer p {
            margin: 0;
            font-size: 0.9em;
        }
    </style>
</head>
<body>
    <header>
        <h1>YOUPILIGHT</h1>
        <nav>
            <ul>
                <li><a href="#reglages">Réglages</a></li>
                <li><a href="#commandes">Commandes</a></li>
            </ul>
        </nav>
    </header>
    <main>
        <section>
            <h2>Heure actuelle</h2>
            <p><span id="heure">--</span> : <span id="minute">--</span> : <span id="seconde">--</span></p>
        </section>
        <section id="reglages">
            <h2>Réglages horaires</h2>
            <form id="config-form">
            <fieldset>
                <legend><strong>Allumage</strong></legend>
                <label for="on_heure">Heure</label><input type="number" id="on_heure" placeholder="Entrer l'heure ici ..." min="0" max="23" required><br>
                <label for="on_minute">Minute</label><input type="number" id="on_minute" placeholder="Entrer la minute ici ..." min="0" max="59" required><br>
                <label for="on_seconde">Seconde</label><input type="number" id="on_seconde" placeholder="Entrer la seconde ici ..." min="0" max="59" required>
            </fieldset>
            <fieldset>
                <legend><strong>Extinction</strong></legend>
                <label for="off_heure">Heure</label><input type="number" id="off_heure" placeholder="Entrer l'heure ici ..." min="0" max="23" required><br>
                <label for="off_minute">Minute</label><input type="number" id="off_minute" placeholder="Entrer la minute ici ..." min="0" max="59" required><br>
                <label for="off_seconde">Seconde</label><input type="number" id="off_seconde" placeholder="Entrer la seconde ici ..." min="0" max="59" required>
            </fieldset>
            <button type="submit" id="btn_reglage">Envoyer le réglage</button>
            </form>
        </section>
        <section id="commandes">
            <h2>Commande de la lampe</h2>
            <article>
                <p>État de la lampe : <span id="etat_lampe"><strong>Éteinte</strong></span></p>
                <button id="off_lampe"  onclick="envoyerEtat('/commands')">Éteindre</button>
            </article>
        </section>
    </main>
    <footer>
        <p>&copy; YoupiLight - 2025 - Tous droits réservés</p>
    </footer>
    <script>
        let heureEl = document.getElementById('heure');
let minuteEl = document.getElementById('minute');
let secondeEl = document.getElementById('seconde');
const url_time = "/time";
let ledState = false;
// ==== GESTION DE L'AFFICHAGE DE L'HEURE ====
// Fonction pour recevoir le temps du module RTC 
async function getTime(url) {
    try {
        const response = await fetch(url);
        if(!response.ok) {
            throw new Error (`Erreur lors de la récupération du temps : ${response.status}`);
        }
        const data  = await response.json();
        //console.log(data);
        if (data) return data;
    } catch (err) {
        console.log("Erreur : ",err.message)
    }
}

// Fonction pour afficher le temps reçu
async function displayTime(data) {
    try {
        const dataObject = await data;
        if (dataObject) {
            let heure = dataObject.heure < 10 ? "0" + dataObject.heure : dataObject.heure;
            let minute = dataObject.minute < 10 ? "0" + dataObject.minute : dataObject.minute;
            let seconde = dataObject.seconde < 10 ? "0" + dataObject.seconde : dataObject.seconde;
            document.getElementById("off_lampe").innerText = dataObject.ledState ? "Eteindre" : "Allumer";
            // Injection des données 
            minuteEl.textContent = minute || "00";
            heureEl.textContent = heure || "00";
            secondeEl.textContent = seconde || "00";            
        }
    }catch (err) {
        console.log("Erreur d'affichage :" ,err.message);
    }
};
setInterval(() => {
    const time = getTime(url_time);
    displayTime(time);
},1000);

// ==== Fin de la section d'affichage du temps ====

    function envoyerEtat(url) {
        console.log("passé");
        fetch(url, {
            method: "POST",
            headers: {
            "Content-Type": "application/json"
            },
            body: JSON.stringify({ etat: !ledState })
        })
        .then(response => {
            if (!response.ok) {
            throw new Error("Erreur HTTP: " + response.status);
            }
            return response.json(); // ou response.text() selon la réponse attendue
        })
        .then(data => {
            console.log("Réponse reçue :", data);
        })
        .catch(error => {
            console.error("Erreur lors de l'envoi :", error);
        });
        }
    </script>
<script>
    
let on_heure = document.getElementById('on_heure');
let on_minute = document.getElementById('on_minute');
let on_seconde = document.getElementById('on_seconde');
let off_heure = document.getElementById('off_heure');
let off_minute = document.getElementById('off_minute');
let off_seconde = document.getElementById('off_seconde');
let etat_lampe = document.getElementById('etat_lampe');
let btnCommande = document.getElementById('btn_commande');
let btnReglage = document.getElementById('btn_reglage');
let configForm = document.getElementById("config-form");


const url_config = "/config";
// ==== Recupération des données du formulaire et envoi ====

if(configForm) {
    configForm.addEventListener('submit', (e)=> {
        e.preventDefault();
       let configElValuesObject = {
            on_heure : parseInputToInt(on_heure.value),
            on_minute : parseInputToInt(on_minute.value),
            on_seconde : parseInputToInt(on_seconde.value),
            off_heure : parseInputToInt(off_heure.value),
            off_minute : parseInputToInt(off_minute.value),
            off_seconde : parseInputToInt(off_seconde.value),
       }
        console.log("Données prêtes à être envoyées : ",configElValuesObject);
        sendConfig(configElValuesObject,url_config);
        configForm.reset();

    });
   
}


function parseInputToInt(data) {
    const parsed = parseInt(data, 10);
    return isNaN(parsed) ? 0 : parsed;
}

async function sendConfig(config,url) {
    const options = {
        method : 'POST',
        headers : {
            'content-type' : 'application/json'
        },
        body : JSON.stringify(config)
    }
    try {
            
        const response = await fetch(url,options);
        if(!response.ok) {
            throw new Error (`Erreur HTTP ! status : ${response.status}`);
        }
        const responseData = await response.json();
        console.log("Données envoyées");
        console.log(responseData);
        if(responseData) return responseData;   
    }
    catch(err) {
        console.log("Erreur lors de l'envoi de la configuration : ",err)
        throw err;
    }
}
</script>
</body>
</html>
)rawliteral";
