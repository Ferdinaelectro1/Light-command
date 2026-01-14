// --- Définition des URLs centralisées ---
const apiUrls = {
    setWifi: () => "/setInfos-wifi",
    setTime: () => "/setTime"
};

// --- Fonctions utilitaires pour l'interface ---
function showFeedbackMessage(element, message, type = 'success') {
    element.textContent = message;
    element.className = `feedback-message ${type}`;
    element.style.display = 'block';
    // Masque le message après 5 secondes
    setTimeout(() => {
        element.style.display = 'none';
    }, 5000);
}

function showLoadingState(button, isLoading) {
    if (isLoading) {
        button.disabled = true;
        button.classList.add('loading');
        button.innerHTML = '<span class="spinner"></span> Chargement...';
    } else {
        button.disabled = false;
        button.classList.remove('loading');
    }
}

// --- Fonctions spécialisées pour les requêtes API ---
const api = {
    async setWifiConfig(ssid, password) {
        const url = apiUrls.setWifi();
        const configData = {
            ssid: ssid,
            password: password
        };
        return await fetchESP(url, configData);
    },

    async setTime(timeData) {
        const url = apiUrls.setTime();
        return await fetchESP(url, timeData);
    }
};

const ssidEl = document.getElementById("ssid");
const passwordEl = document.getElementById("password");
const timeEl = document.getElementById("time");
const wifiEl = document.getElementById("wifi-form");
const hourFormEl = document.getElementById('hour-form');
const wifiBtn = document.getElementById("wifi-btn-modify");
const timeBtn = document.getElementById("time-btn-modify");
const wifiSection = document.getElementById('wifiSection');
const timeSection = document.getElementById('timeSection');
const passwordToggle = document.getElementById('password-toggle');
const wifiMessageEl = document.getElementById('wifi-message');
const timeMessageEl = document.getElementById('time-message');
const modalSections = document.querySelectorAll('.modal');

if (!ssidEl || !passwordEl || !timeEl || !wifiEl || !hourFormEl || !wifiBtn || !timeBtn || !timeSection || !wifiSection || !passwordToggle) {
    console.error("Un ou plusieurs éléments n'existent pas dans le DOM. Le script ne peut pas continuer.");
} else {
    // ⛱️ Masquer tout au départ
    modalSections.forEach(section => section.style.display = "none");

    // Fonction utilitaire pour afficher une modale et cacher les autres
    function showModal(modalToShow) {
        modalSections.forEach(section => {
            section.style.display = "none";
            // Cache les messages précédents
            const messageEl = section.querySelector('.feedback-message');
            if (messageEl) messageEl.style.display = 'none';
        });
        modalToShow.style.display = "block";
    }

    // 📌 Bouton Wi-Fi
    wifiBtn.addEventListener("click", () => {
        showModal(wifiSection);
    });

    // 📌 Bouton Heure
    timeBtn.addEventListener("click", () => {
        showModal(timeSection);
    });
    
    // 💡 Fonctionnalité d'affichage/masquage du mot de passe
    passwordToggle.addEventListener("click", () => {
        const type = passwordEl.getAttribute("type") === "password" ? "text" : "password";
        passwordEl.setAttribute("type", type);
        passwordToggle.classList.toggle("visible");
    });

    // 📡 Événement sur la soumission du formulaire Wi-Fi
    wifiEl.addEventListener("submit", async (e) => {
        e.preventDefault();
        const button = e.submitter;
        const ssid = ssidEl.value.trim();
        const password = passwordEl.value.trim();

        if (password.length < 8) {
            showFeedbackMessage(wifiMessageEl, "Le mot de passe doit avoir au moins 8 caractères.", 'error');
            return;
        }

        if (ssid === "" || password === "") {
            showFeedbackMessage(wifiMessageEl, "La configuration est vide !", 'error');
            return;
        }

        showLoadingState(button, true);
        const responseData = await api.setWifiConfig(ssid, password);
        showLoadingState(button, false);
        
        if (responseData && responseData.success) {
            showFeedbackMessage(wifiMessageEl, "Configuration Wi-Fi mise à jour avec succès !", 'success');
            wifiEl.reset();
        } else {
            showFeedbackMessage(wifiMessageEl, "Échec de l'envoi de la configuration Wi-Fi.", 'error');
        }
    });
    
    // ⏰ Événement sur la soumission du formulaire Heure
    hourFormEl.addEventListener("submit", async (e) => {
        e.preventDefault();
        const button = e.submitter;
        const timeData = parseInput(timeEl);

        if (!timeData) {
            showFeedbackMessage(timeMessageEl, "Données de temps invalides.", 'error');
            return;
        }
        
        showLoadingState(button, true);
        const responseData = await api.setTime(timeData);
        showLoadingState(button, false);
        
        if (responseData && responseData.success) {
            showFeedbackMessage(timeMessageEl, "Heure réglée avec succès !", 'success');
            hourFormEl.reset();
        } else {
            showFeedbackMessage(timeMessageEl, "Échec de l'envoi de la configuration de l'heure.", 'error');
        }
    });
}

function parseInput(input) {
    if (!input || input.value.trim() === "") {
        console.error("Données invalides pour être parsé");
        return null;
    }
    const [hour, minute] = input.value.trim().split(':').map(n => parseInt(n, 10));
    return {
        heure: hour,
        minute: minute,
        seconde: 0
    };
}

async function fetchESP(url, data = null) {
    const options = data
        ? {
            method: "POST",
            headers: { "content-type": "application/json" },
            body: JSON.stringify(data)
          }
        : { method: "GET" };

    try {
        const response = await fetch(url, options);
        if (!response.ok) {
            throw new Error(`Erreur HTTP ! ${response.status}`);
        }

        const responseData = await response.json();
        console.log("Réponse du serveur : ", responseData);
        // Ajout d'une propriété `success` pour simplifier la vérification
        return { ...responseData, success: true };
    } catch (err) {
        console.error("Erreur lors du fetch avec l'esp !", err);
        return { success: false, error: err.message };
    }
}