// Définition des URLs centralisées (comme on l'a vu)
const apiUrls = {
  getSaveState: (sortie) => `/getSaveState/sortie-${sortie}`,
  setSaveState: (sortie) => `/save/sortie-${sortie}`,
  sendCalendar: (sortie, tache) => `/sortie-${sortie}/tache-${tache}`
};

// Fonctions spécialisées pour les requêtes API
const api = {
  async getSaveState(sortie) {
    const url = apiUrls.getSaveState(sortie);
    const response = await fetchESP(url);
    if (response?.data && typeof response.data.save === "boolean") {
      return response.data.save;
    }
    console.warn(`Structure inattendue du serveur pour l'état de sauvegarde de la sortie ${sortie}`);
    return null;
  },

  async setSaveState(sortie, choice) {
    const url = apiUrls.setSaveState(sortie);
    const data = { save: choice };
    const response = await fetchESP(url, data);
    if (response?.data && typeof response.data.save === "boolean") {
      return response.data.save;
    }
    console.warn(`Échec de la sauvegarde pour la sortie ${sortie}`);
    return null;
  },

  async sendCalendar(sortie, tache, calendarData) {
    const url = apiUrls.sendCalendar(sortie, tache);
    const response = await fetchESP(url, calendarData);
    if (response?.data) {
      return response.data;
    }
    console.warn(`Échec de l'envoi de la tâche pour la sortie ${sortie}`);
    return null;
  }
};

// --- Reste du code ---
document.addEventListener('DOMContentLoaded', () => {
  const form = document.getElementById('sortie-form');
  const saveForm = document.getElementById('save-form');
  const sortieEL = document.getElementById('sorties');
  const tacheEL = document.getElementById('taches');
  const saveButton = document.getElementById('btn-save');
  const saveChoiceLabel = document.getElementById('choice_state');
  const selectedOutputSpan = document.getElementById('selected-output');

  // 🔄 Initialisation au chargement
  const selectedSortie = sortieEL.value;
  updateTextContent(selectedOutputSpan, sortieEL.options[sortieEL.selectedIndex].text);
  // 💡 Appel à la nouvelle fonction api.getSaveState
  updateSaveStateUI(selectedSortie);

  // 🧭 Écoute de changement de sortie
  sortieEL.addEventListener('change', (e) => {
    const sortieChoice = e.target.value;
    updateTextContent(selectedOutputSpan, sortieEL.options[sortieEL.selectedIndex].text);
    // 💡 Appel à la nouvelle fonction api.getSaveState
    updateSaveStateUI(sortieChoice);
  });

  // 📅 Soumission du formulaire de tâche
  form.addEventListener('submit', async (e) => {
    e.preventDefault();
    const sortie = sortieEL.value;
    const tache = tacheEL.value;
    const onTime = document.getElementById('onTime').value;
    const offTime = document.getElementById('offTime').value;
    const data = timeDataset(onTime, offTime);
    // 💡 Appel à la nouvelle fonction api.sendCalendar
    const responseData = await api.sendCalendar(sortie, tache, data);
    if (responseData) {
      console.log("Réponse tâche :", responseData);
    }
    form.reset();
  });

  // 💾 Soumission du formulaire de sauvegarde
  saveForm.addEventListener('submit', async (e) => {
    e.preventDefault();
    const sortie = sortieEL.value;
    const saveChoice = saveButton.checked;
    // 💡 Appel à la nouvelle fonction api.setSaveState
    await updateSetSaveStateUI(sortie, saveChoice);
  });
});

// Fonctions pour gérer la mise à jour de l'interface utilisateur
async function updateSaveStateUI(sortie) {
  const checkbox = document.getElementById('btn-save');
  const choiceLabel = document.getElementById('choice_state');

  // Appel à la fonction spécialisée pour obtenir l'état de sauvegarde
  const saveState = await api.getSaveState(sortie);

  if (saveState !== null) {
    checkbox.checked = saveState;
    updateTextContent(choiceLabel, saveState ? "Désactiver" : "Activer");
  } else {
    checkbox.checked = false;
    updateTextContent(choiceLabel, "---");
  }
}

async function updateSetSaveStateUI(sortie, saveChoice) {
  const label = document.getElementById('choice_state');

  // Appel à la fonction spécialisée pour définir l'état de sauvegarde
  const newSaveState = await api.setSaveState(sortie, saveChoice);

  if (newSaveState !== null) {
    updateTextContent(label, newSaveState ? "Désactiver" : "Activer");
    console.log(`Sauvegarde pour sortie ${sortie} :`, newSaveState);
  } else {
    updateTextContent(label, "---");
  }
}


function updateTextContent(element, text) {
  element.textContent = text;
}

function parseInputTime(time) {
  const [h, m] = time.split(':').map(n => parseInt(n, 10));
  return { heure: h, minute: m, seconde: 0 };
}

function timeDataset(on_time, off_time) {
  return {
    allumage: parseInputTime(on_time),
    extinction: parseInputTime(off_time)
  };
}

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
    if (!response.ok) throw new Error(`HTTP ${response.status}`);
    const json = await response.json();
    console.log("Réponse ESP :", json);
    return json;
  } catch (err) {
    console.error("Erreur ESP :", err);
    return null;
  }
}