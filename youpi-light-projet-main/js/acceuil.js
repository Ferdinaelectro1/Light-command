// --- Définition des URLs centralisées ---
const apiUrls = {
    getDate: () => "/getDate",
    getTime: () => "/getTime",
    getWifiStatus: () => "/getWifiStatus"
};

// --- Fonctions utilitaires pour l'interface ---

// 💡 Fonction utilitaire pour ajouter un zéro si nécessaire
function formatNumber(number) {
    const num = parseInt(number, 10);
    return num < 10 ? `0${num}` : num;
}

// --- Fonctions spécialisées pour les requêtes API ---
const api = {
    async getTime() {
        const url = apiUrls.getTime();
        const response = await fetchESP(url);
        return response?.data ?? null;
    },
    
    async getDate() {
        const url = apiUrls.getDate();
        const response = await fetchESP(url);
        return response?.data ?? null;
    },
    
    async getWifiStatus() {
        const url = apiUrls.getWifiStatus();
        const response = await fetchESP(url);
        return response?.data?.isConnected ?? null;
    }
};

// --- Reste du code ---
const hourEl = document.getElementById('hour');
const minuteEl = document.getElementById('minute');
const secondeEl = document.getElementById('seconde');
const yearEl = document.getElementById('year');
const monthEl = document.getElementById('month');
const dayEl = document.getElementById('day');
const networkStatusEl = document.getElementById('network-status');

let timeUpdateInterval;

document.addEventListener('DOMContentLoaded', () => {
    acceuilInit();
});

// Fonction pour l'initialisation de la page
function acceuilInit() {
    updateDateAndStatus();
    scheduleTimeUpdate();
}

// 📆 Mise à jour de la date et du statut
async function updateDateAndStatus() {
    const dateData = await api.getDate();
    const isConnected = await api.getWifiStatus();
    
    updateDateUI(dateData);
    updateNetworkStatusUI(isConnected);
}

// ⌚ Lancement de la mise à jour de l'heure
function scheduleTimeUpdate() {
    // Annule tout intervalle existant pour éviter les doublons
    if (timeUpdateInterval) {
        clearInterval(timeUpdateInterval);
    }
    // Met à jour l'heure immédiatement au chargement
    updateTimeUI();
    // Met à jour l'heure toutes les 5 secondes (5000 ms)
    timeUpdateInterval = setInterval(updateTimeUI, 5000);
}

// ⌚ Mise à jour de l'UI de l'heure
async function updateTimeUI() {
    const timeData = await api.getTime();
    
    if (timeData) {
        let { heure, minute, seconde } = timeData;
        hourEl.textContent = formatNumber(heure);
        minuteEl.textContent = formatNumber(minute);
        secondeEl.textContent = formatNumber(seconde);
    } else {
        // En cas d'erreur, on affiche un message clair
        console.error("Échec de la récupération de l'heure depuis le système.");
        hourEl.textContent = "--";
        minuteEl.textContent = "--";
        secondeEl.textContent = "--";
    }
}

// 📆 Mise à jour de l'UI de la date
function updateDateUI(dateData) {
    if (dateData) {
        const { annee, mois, jour } = dateData;
        yearEl.textContent = annee;
        monthEl.textContent = formatNumber(mois);
        dayEl.textContent = formatNumber(jour);
    } else {
        console.error("Aucune donnée du serveur pour la date.");
        yearEl.textContent = "--";
        monthEl.textContent = "--";
        dayEl.textContent = "--";
    }
}

// 🔌 Mise à jour de l'UI du statut réseau
function updateNetworkStatusUI(isConnected) {
    const statusTextEl = networkStatusEl.querySelector('strong');
    
    if (isConnected === true) {
        statusTextEl.textContent = "Connecté";
        statusTextEl.classList.remove('disconnected');
        statusTextEl.classList.add('connected');
    } else if (isConnected === false) {
        statusTextEl.textContent = "Déconnecté";
        statusTextEl.classList.remove('connected');
        statusTextEl.classList.add('disconnected');
    } else {
        statusTextEl.textContent = "Indisponible";
        statusTextEl.classList.remove('connected', 'disconnected');
    }
}

// 🔌 Requête générique vers l’ESP
async function fetchESP(url, data = null) {
    const options = data
        ? {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify(data)
          }
        : { method: "GET" };

    try {
        const response = await fetch(url, options);
        if (!response.ok) {
            throw new Error(`Erreur HTTP! ${response.status}`);
        }
        return await response.json();
    } catch (err) {
        console.error(`Erreur fetchESP (${url}):`, err);
        return null;
    }
}