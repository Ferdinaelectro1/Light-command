// --- Définition des URLs centralisées ---
const apiUrls = {
  getDate: () => "/getDate",
  getTime: () => "/getTime"
};

// --- Fonctions spécialisées pour les requêtes API ---
const api = {
  async getDate() {
    const url = apiUrls.getDate();
    const response = await fetchESP(url);
    if (response?.data) {
      return response.data;
    }
    console.warn("Réponse inattendue du serveur pour la date :", response);
    return null;
  },

  async getTime() {
    const url = apiUrls.getTime();
    const response = await fetchESP(url);
    if (response?.data) {
      return response.data;
    }
    console.warn("Réponse inattendue du serveur pour l'heure :", response);
    return null;
  }
};

// --- Reste du code ---
const hourEl = document.getElementById('hour');
const minuteEl = document.getElementById('minute');
const secondeEl = document.getElementById('seconde');
const yearEl = document.getElementById('year');
const monthEl = document.getElementById('month');
const dayEl = document.getElementById('day');

document.addEventListener('DOMContentLoaded', () => {
  acceuilInit();
});

function acceuilInit() {
  scheduleDateUpdate(); // Lancement cyclique de la date
  scheduleTimeUpdate(); // Lancement cyclique de l’heure
}

// ⏳ Mise à jour toutes les 12h (43_200_000 ms)
function scheduleDateUpdate() {
  updateDateUI().finally(() => {
    setTimeout(scheduleDateUpdate, 43200000);
  });
}

// ⏱️ Mise à jour toutes les secondes
function scheduleTimeUpdate() {
  updateTimeUI().finally(() => {
    setTimeout(scheduleTimeUpdate, 1000);
  });
}

// 📆 Mise à jour de l'UI de la date
async function updateDateUI() {
  const dateData = await api.getDate();

  if (dateData) {
    const { annee, mois, jour } = dateData;
    console.log("Données reçues pour la date :", dateData);
    yearEl.textContent = annee ?? "--";
    monthEl.textContent = mois ?? "--";
    dayEl.textContent = jour ?? "--";
  } else {
    console.log("Aucune donnée du serveur pour la date. Affichage par défaut.");
    yearEl.textContent = "--";
    monthEl.textContent = "--";
    dayEl.textContent = "--";
  }
}

// ⌚ Mise à jour de l'UI de l’heure
async function updateTimeUI() {
  const timeData = await api.getTime();

  if (timeData) {
    const { heure, minute, seconde } = timeData;
    console.log("Données reçues pour l’heure :", timeData);
    hourEl.textContent = heure ?? "--";
    minuteEl.textContent = minute ?? "--";
    secondeEl.textContent = seconde ?? "--";
  } else {
    console.log("Aucune donnée du serveur pour l'heure. Affichage par défaut.");
    hourEl.textContent = "--";
    minuteEl.textContent = "--";
    secondeEl.textContent = "--";
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
    if (!response.ok) throw new Error(`Erreur HTTP! ${response.status}`);
    return await response.json();
  } catch (err) {
    console.error(`Erreur fetchESP (${url}):`, err);
    return null;
  }
}