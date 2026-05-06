# C++ Order Book - Interactive Documentation

This directory contains interactive HTML documentation for the C++ Order Book project.

## Files

- **index.html** - Main interactive documentation with Mermaid diagrams
- **mermaid-height-util.js** - Utility for diagram scaling and tab management
- **README.md** - This file

## Usage

### Opening the Documentation

Simply open `index.html` in a web browser:

```bash
# Using file:// protocol (works offline after initial load)
start docs/index.html

# Or with a local server
python -m http.server 8000
# Then navigate to http://localhost:8000/docs/
```

### Requirements

- Modern web browser (Chrome, Firefox, Safari, Edge)
- Internet connection (for loading Mermaid.js from CDN on first load)

## Features

### Interactive Tabs

The documentation is organized into six main sections:

1. **📊 Overview** - System overview, capabilities, and performance benchmarks
2. **🏗️ Architecture** - Component architecture and design patterns
3. **🔄 API** - Complete API reference with usage examples
4. **🗄️ Data Structures** - Internal data structures and relationships
5. **🛡️ Memory Safety** - Smart pointer migration and safety guarantees
6. **🚀 Deployment** - Build instructions and deployment guide

### Mermaid Diagrams

Each section includes interactive Mermaid diagrams:

- **Overview**: High-level system architecture graph
- **Architecture**: Sequence diagram showing component interaction
- **API**: Flowchart of order processing pipeline
- **Data Structures**: Entity-relationship diagram
- **Memory Safety**: Flowchart of smart pointer migration
- **Deployment**: Build and deployment pipeline

### Diagram Controls

Each diagram includes zoom controls:

- **−** - Zoom out (10% decrement)
- **+** - Zoom in (10% increment)
- **⟲** - Reset to 100% scale
- Scale range: 50% - 200%

### Responsive Design

The documentation is fully responsive:

- Desktop: Horizontal tab navigation
- Mobile: Vertical tab navigation with optimized controls
- Diagrams support both horizontal and vertical scrolling

## Technical Details

### Mermaid Configuration

```javascript
{
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
}
```

### Color Scheme

- **Primary components**: `#667eea` (purple-blue gradient)
- **Success states**: `#28a745` (green)
- **Warnings**: `#ffc107` (yellow)
- **Errors/Critical**: `#dc3545` (red)
- **Auxiliary**: `#6c757d` (gray)

### Browser Compatibility

- Chrome 90+
- Firefox 88+
- Safari 14+
- Edge 90+

## Offline Usage

After the initial load, the documentation can be used offline:

1. Open `index.html` once with an internet connection to load Mermaid.js
2. The cached version will work offline
3. For fully offline usage, download `mermaid.min.js` and update the script tag

## Customization

### Adding New Sections

To add a new section:

1. Add a tab button in the `.nav-tabs` container
2. Add a `.tab-content` div with corresponding ID
3. Include a Mermaid diagram with unique ID
4. The tab switching logic will handle the rest

### Modifying Diagrams

Edit the Mermaid diagram code within the `.mermaid` divs. Reference the [Mermaid documentation](https://mermaid.js.org/) for syntax.

### Styling

All styles are inline in the `<style>` tag within `<head>`. Modify as needed for custom branding.

## Related Documentation

- [API Documentation](API.md) - Detailed API reference
- [Performance Analysis](PERFORMANCE.md) - Benchmark results
- [Smart Pointer Migration](SMART_POINTER_MIGRATION.md) - Migration guide
- [Project Structure](PROJECT_STRUCTURE.md) - Architecture details
- [Build Instructions](BUILD.md) - Build system guide

## Support

For issues or questions about the documentation:

- Check the main project [README.md](../README.md)
- Review the [GitHub Issues](https://github.com/revitalyr/Analyze_Legacy_Trading_Engine/issues)
- Consult the [Engineering Notes](../README_ENGINEERING.md)

## License

This documentation is part of the C++ Order Book project, licensed under the MIT License. See the [LICENSE](../LICENSE) file for details.
