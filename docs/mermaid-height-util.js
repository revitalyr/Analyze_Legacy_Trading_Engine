class MermaidHeightAdjuster {
    constructor() {
        this.scale = 100;
        this.scaleFactors = new Map();
        this.init();
    }

    async init() {
        await mermaid.initialize({
            startOnLoad: false,
            theme: 'default',
            themeVariables: {
                primaryColor: '#667eea',
                primaryTextColor: '#333',
                lineColor: '#667eea',
                secondaryColor: '#764ba2',
                fontSize: '14px'
            },
            flowchart: { useMaxWidth: false, htmlLabels: true }
        });

        this.setupTabSwitching();
        this.setupScaleControls();
    }

    setupTabSwitching() {
        const tabs = document.querySelectorAll('.nav-tab');
        const contents = document.querySelectorAll('.tab-content');

        tabs.forEach(tab => {
            tab.addEventListener('click', () => {
                const targetTab = tab.dataset.tab;

                tabs.forEach(t => t.classList.remove('active'));
                contents.forEach(c => c.classList.remove('active'));

                tab.classList.add('active');
                const targetContent = document.getElementById(targetTab);
                if (targetContent) {
                    targetContent.classList.add('active');

                    const diagram = targetContent.querySelector('.mermaid');
                    if (diagram && !diagram.hasAttribute('data-processed')) {
                        setTimeout(async () => {
                            await mermaid.run({
                                nodes: [diagram]
                            });
                            const container = diagram.closest('.diagram-container');
                            if (container) {
                                this.adjustDiagram(container);
                                this.applyScale(container);
                            }
                        }, 100);
                    } else if (diagram) {
                        const container = diagram.closest('.diagram-container');
                        if (container) {
                            this.applyScale(container);
                        }
                    }
                }
            });
        });

        // Render first tab on load
        const firstTab = document.querySelector('.nav-tab.active');
        if (firstTab) {
            firstTab.click();
        }
    }

    setupScaleControls() {
        document.addEventListener('click', (e) => {
            const scaleBtn = e.target.closest('[data-scale-delta]');
            const resetBtn = e.target.closest('[data-reset-scale]');
            const container = e.target.closest('.diagram-container');

            if (scaleBtn && container) {
                const delta = parseInt(scaleBtn.dataset.scaleDelta);
                this.changeScale(delta, container);
            }

            if (resetBtn && container) {
                this.resetScale(container);
            }
        });
    }

    adjustDiagram(container) {
        const svg = container.querySelector('svg');
        if (!svg) return;

        const bbox = svg.getBBox();
        const padding = 20;

        svg.style.width = '100%';
        svg.style.height = 'auto';
        svg.style.minWidth = `${bbox.width + padding * 2}px`;
        svg.style.minHeight = `${bbox.height + padding * 2}px`;
    }

    changeScale(delta, container) {
        const containerId = this.getContainerId(container);
        const currentScale = this.scaleFactors.get(containerId) || 100;
        const newScale = Math.max(50, Math.min(200, currentScale + delta));

        this.scaleFactors.set(containerId, newScale);
        this.applyScale(container);
    }

    resetScale(container) {
        const containerId = this.getContainerId(container);
        this.scaleFactors.set(containerId, 100);
        this.applyScale(container);
    }

    applyScale(container) {
        const containerId = this.getContainerId(container);
        const scale = this.scaleFactors.get(containerId) || 100;
        const svg = container.querySelector('svg');

        if (svg) {
            svg.style.transform = `scale(${scale / 100})`;
            svg.style.transformOrigin = 'top left';
            svg.style.transition = 'transform 0.2s ease';
        }

        this.updateScaleDisplay(container, scale);
    }

    updateScaleDisplay(container, scale) {
        const display = container.querySelector('.scale-display');
        if (display) {
            display.textContent = `${scale}%`;
        }
    }

    getContainerId(container) {
        return Array.from(document.querySelectorAll('.diagram-container')).indexOf(container);
    }
}

// Initialize on DOM load
document.addEventListener('DOMContentLoaded', () => {
    new MermaidHeightAdjuster();
});
