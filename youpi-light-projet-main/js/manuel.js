// --- Définition des URLs centralisées ---
const apiUrls = {
    getState: (num) => `/sortie-${num}/getState`,
    setState: (num) => `/sortie-${num}/setState`,
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
        button.innerHTML = '<span class="spinner"></span>';
    } else {
        button.disabled = false;
        button.classList.remove('loading');
    }
}

function updateButtonText(button, state) {
    if (state === null) {
        button.textContent = "---";
    } else {
        button.textContent = state ? "Désactiver" : "Activer";
    }
}

// 🔌 Fonction générique pour interagir avec l’ESP
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
        if (!response.ok) throw new Error(`Erreur HTTP ${response.status}`);
        const responseData = await response.json();
        return { ...responseData, success: true };
    } catch (err) {
        console.error("❌ Erreur fetchESP :", err);
        return { success: false, error: err.message };
    }
}

// --- Fonctions spécialisées pour les requêtes API ---
const api = {
    async getState(num) {
        const url = apiUrls.getState(num);
        const response = await fetchESP(url);
        if (response.success && response?.data?.state !== undefined) {
            return response.data.state;
        }
        console.warn(`⚠️ Réponse inattendue pour l'état de la sortie ${num}`);
        return null;
    },
    
    async setState(num, newState) {
        const url = apiUrls.setState(num);
        const response = await fetchESP(url, { state: newState });
        if (response.success && response?.data?.state !== undefined) {
            return response.data.state;
        }
        console.warn(`⚠️ Réponse inattendue pour la commande de la sortie ${num}`);
        return null;
    }
};

// --- Reste du code ---
document.addEventListener('DOMContentLoaded', () => {
    const sorties = [1, 2];
    const updateIntervals = {};
    const updateFrequency = 1000; // Fréquence d'actualisation en ms

    sorties.forEach(num => setupSortie(num, updateFrequency));

    function setupSortie(num, frequency) {
        const stateEl = document.getElementById(`sortie-${num}-state`);
        const button = document.getElementById(`sortie-${num}-button`);
        const messageEl = document.getElementById(`sortie-${num}-message`);

        if (!stateEl || !button || !messageEl) {
            console.error(`❌ Éléments HTML manquants pour la sortie ${num}`);
            return;
        }

        async function updateUI() {
            const state = await api.getState(num);
            
            if (state !== null) {
                stateEl.textContent = state ? "Active" : "Inactive";
                updateButtonText(button, state);
                button.dataset.state = state;
            } else {
                stateEl.textContent = "---";
                updateButtonText(button, null);
                showFeedbackMessage(messageEl, "Échec de l'actualisation de l'état.", "error");
            }
        }

        function startUpdateInterval() {
            if (updateIntervals[num]) {
                clearInterval(updateIntervals[num]);
            }
            updateIntervals[num] = setInterval(updateUI, frequency);
        }

        button.addEventListener('click', async () => {
            showLoadingState(button, true);
            clearInterval(updateIntervals[num]);
            
            const nouvelEtat = !(button.dataset.state === 'true');
            const sentState = await api.setState(num, nouvelEtat);
            
            if (sentState !== null) {
                // 💡 On met à jour l'UI avec la nouvelle valeur après un court délai
                // pour laisser le temps au serveur de changer l'état
                setTimeout(async () => {
                    await updateUI();
                    showFeedbackMessage(messageEl, `État de la sortie ${num} mis à jour avec succès !`, "success");
                    showLoadingState(button, false);
                    startUpdateInterval();
                }, 500); 
            } else {
                showFeedbackMessage(messageEl, `Échec de la mise à jour de la sortie ${num}.`, "error");
                showLoadingState(button, false);
                startUpdateInterval();
            }
        });

        updateUI();
        startUpdateInterval();
    }
});