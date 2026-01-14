// Définition des URLs centralisées
const apiUrls = {
    getCalendarConfigs: () => `/getCalendarConfigs`,
    getSaveState: (sortie) => `/getSaveState/sortie-${sortie}`,
    setSaveState: (sortie) => `/save/sortie-${sortie}`,
    sendCalendar: (sortie, tache) => `/sortie-${sortie}/tache-${tache}`
};

// --- Fonctions utilitaires pour l'interface utilisateur ---
function showFeedbackMessage(element, message, type = 'success') {
    element.textContent = message;
    element.className = `feedback-message ${type} visible`;
    setTimeout(() => {
        element.classList.remove('visible');
        element.textContent = "";
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
        button.innerHTML = button.id === 'send-calendar-btn' ? 'Envoyer le réglage' : 'Appliquer le changement';
    }
}

function updateButtonText(button, text) {
    button.textContent = text;
}

function updateTextContent(element, text) {
    element.textContent = text;
}

// Fonctions spécialisées pour les requêtes API
const api = {
    async getCalendarConfigs() {
        const url = apiUrls.getCalendarConfigs();
        return await fetchESP(url);
    },
    async getSaveState(sortie) {
        const url = apiUrls.getSaveState(sortie);
        return await fetchESP(url);
    },
    async setSaveState(sortie, choice) {
        const url = apiUrls.setSaveState(sortie);
        const data = { save: choice };
        return await fetchESP(url, data);
    },
    async sendCalendar(sortie, tache, calendarData) {
        const url = apiUrls.sendCalendar(sortie, tache);
        return await fetchESP(url, calendarData);
    }
};

// Éléments du DOM
const sortieEL = document.getElementById('sorties');
const tacheEL = document.getElementById('taches');
const onTimeEl = document.getElementById('onTime');
const offTimeEl = document.getElementById('offTime');
const saveButton = document.getElementById('btn-save');
const saveChoiceLabel = document.getElementById('choice_state');
const selectedOutputSpan = document.getElementById('selected-output');
const calendarFormEl = document.getElementById('sortie-form');
const saveFormEl = document.getElementById('save-form');
const calendarMessageEl = document.getElementById('calendar-message');
const saveMessageEl = document.getElementById('save-message');
const sendCalendarBtn = document.getElementById('send-calendar-btn');
const sendSaveBtn = document.getElementById('send-save-btn');
const configurationsContainer = document.getElementById('configurations-container');
const refreshCalendarBtn = document.getElementById('refresh-calendar-btn');

document.addEventListener('DOMContentLoaded', () => {
    // Initialisation au chargement
    const selectedSortie = sortieEL.value;
    updateTextContent(selectedOutputSpan, sortieEL.options[sortieEL.selectedIndex].text);
    updateSaveStateUI(selectedSortie);
    updateCalendarConfigsUI(); // Afficher les configurations au démarrage

    // Écoute des événements
    sortieEL.addEventListener('change', (e) => {
        const sortieChoice = e.target.value;
        updateTextContent(selectedOutputSpan, sortieEL.options[sortieEL.selectedIndex].text);
        updateSaveStateUI(sortieChoice);
    });

    calendarFormEl.addEventListener('submit', async (e) => {
        e.preventDefault();
        showLoadingState(sendCalendarBtn, true);
        
        const sortie = sortieEL.value;
        const tache = tacheEL.value;
        const onTime = onTimeEl.value;
        const offTime = offTimeEl.value;
        const data = timeDataset(onTime, offTime);
        
        const response = await api.sendCalendar(sortie, tache, data);
        showLoadingState(sendCalendarBtn, false);
        
        if (response && response.success) {
            showFeedbackMessage(calendarMessageEl, "Réglage de la tâche envoyé avec succès !", "success");
            calendarFormEl.reset();
            updateCalendarConfigsUI(); // Mettre à jour l'affichage après un envoi réussi
        } else {
            showFeedbackMessage(calendarMessageEl, "Échec de l'envoi du réglage de la tâche.", "error");
        }
    });

    saveFormEl.addEventListener('submit', async (e) => {
        e.preventDefault();
        showLoadingState(sendSaveBtn, true);
        
        const sortie = sortieEL.value;
        const saveChoice = saveButton.checked;
        const response = await api.setSaveState(sortie, saveChoice);
        showLoadingState(sendSaveBtn, false);
        
        if (response && response.success) {
            showFeedbackMessage(saveMessageEl, "État de sauvegarde mis à jour avec succès !", "success");
            updateTextContent(saveChoiceLabel, response.data.save ? "Désactiver" : "Activer");
        } else {
            showFeedbackMessage(saveMessageEl, "Échec de la mise à jour de l'état de sauvegarde.", "error");
        }
    });

    refreshCalendarBtn.addEventListener('click', () => {
        updateCalendarConfigsUI();
        showFeedbackMessage(calendarMessageEl, "Configurations actualisées.", "success");
    });
});

// Fonctions pour gérer la mise à jour de l'interface utilisateur
async function updateSaveStateUI(sortie) {
    const response = await api.getSaveState(sortie);
    
    if (response && response.data && typeof response.data.save === "boolean") {
        saveButton.checked = response.data.save;
        updateTextContent(saveChoiceLabel, response.data.save ? "Désactiver" : "Activer");
    } else {
        saveButton.checked = false;
        updateTextContent(saveChoiceLabel, "---");
        console.error(`Impossible de récupérer l'état de sauvegarde pour la sortie ${sortie}`);
    }
}

async function updateCalendarConfigsUI() {
    const configsResponse = await api.getCalendarConfigs();
    if (configsResponse.success && configsResponse.data) {
        renderCalendarConfigs(configsResponse.data);
    } else {
        configurationsContainer.innerHTML = `<p class="empty-state">Échec du chargement des configurations.</p>`;
    }
}

function renderCalendarConfigs(configs) {
    let htmlContent = '';
    const hasConfigs = Object.values(configs).some(sortieConfigs => Object.values(sortieConfigs).some(config => config.allumage.heure !== 255));

    if (!hasConfigs) {
        configurationsContainer.innerHTML = `<p class="empty-state">Aucune configuration de calendrier n'est active pour le moment.</p>`;
        return;
    }

    for (const [sortieKey, taches] of Object.entries(configs)) {
        htmlContent += `<div class="card calendar-card"><h3>${sortieKey.charAt(0).toUpperCase() + sortieKey.slice(1)}</h3><ul>`;
        for (const [tacheKey, config] of Object.entries(taches)) {
            // L'ESP envoie 255 pour indiquer une tâche vide.
            if (config.allumage.heure !== 255) {
                const onTime = formatTime(config.allumage.heure, config.allumage.minute);
                const offTime = formatTime(config.extinction.heure, config.extinction.minute);
                htmlContent += `<li><strong>${tacheKey.charAt(0).toUpperCase() + tacheKey.slice(1)}:</strong> Allumage à ${onTime}, Extinction à ${offTime}</li>`;
            }
        }
        htmlContent += `</ul></div>`;
    }
    configurationsContainer.innerHTML = htmlContent;
}

function formatTime(h, m) {
    return `${h.toString().padStart(2, '0')}:${m.toString().padStart(2, '0')}`;
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
        // Ajout d'une propriété `success` pour simplifier la vérification
        return { ...json, success: true };
    } catch (err) {
        console.error("Erreur ESP :", err);
        return { success: false, error: err.message };
    }
}