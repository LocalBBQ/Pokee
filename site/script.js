const header = document.querySelector("[data-header]");
const navToggle = document.querySelector(".nav-toggle");
const navigation = document.querySelector(".site-nav");

const setHeaderState = () => {
  header?.classList.toggle("scrolled", window.scrollY > 16);
};

setHeaderState();
window.addEventListener("scroll", setHeaderState, { passive: true });

navToggle?.addEventListener("click", () => {
  const open = navToggle.getAttribute("aria-expanded") === "true";
  navToggle.setAttribute("aria-expanded", String(!open));
  navigation?.classList.toggle("open", !open);
});

navigation?.querySelectorAll("a").forEach((link) => {
  link.addEventListener("click", () => {
    navToggle?.setAttribute("aria-expanded", "false");
    navigation?.classList.remove("open");
  });
});

const tabs = Array.from(document.querySelectorAll('[role="tab"]'));

const activateTab = (tab) => {
  tabs.forEach((candidate) => {
    const selected = candidate === tab;
    candidate.setAttribute("aria-selected", String(selected));
    candidate.tabIndex = selected ? 0 : -1;
    const panel = document.getElementById(candidate.getAttribute("aria-controls"));
    if (panel) panel.hidden = !selected;
  });
};

tabs.forEach((tab, index) => {
  tab.addEventListener("click", () => activateTab(tab));
  tab.addEventListener("keydown", (event) => {
    if (!['ArrowLeft', 'ArrowRight', 'Home', 'End'].includes(event.key)) return;
    event.preventDefault();
    let nextIndex = index;
    if (event.key === 'ArrowLeft') nextIndex = (index - 1 + tabs.length) % tabs.length;
    if (event.key === 'ArrowRight') nextIndex = (index + 1) % tabs.length;
    if (event.key === 'Home') nextIndex = 0;
    if (event.key === 'End') nextIndex = tabs.length - 1;
    activateTab(tabs[nextIndex]);
    tabs[nextIndex].focus();
  });
});

const versionLabel = document.getElementById("release-version");
const releaseButton = document.getElementById("release-button");

fetch("https://api.github.com/repos/TeamGDB/Yakumo/releases/latest", {
  headers: { Accept: "application/vnd.github+json" },
})
  .then((response) => {
    if (!response.ok) throw new Error("Release lookup failed");
    return response.json();
  })
  .then((release) => {
    if (!release.tag_name) return;
    versionLabel.textContent = `Stable ${release.tag_name}`;
    releaseButton.textContent = `Download ${release.tag_name}`;
  })
  .catch(() => {
    // The static fallback stays useful when GitHub's unauthenticated API is unavailable.
  });

document.getElementById("year").textContent = new Date().getFullYear();
