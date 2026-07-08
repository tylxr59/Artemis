.pragma library

function localImageUrl(path) {
    return path.length > 0 ? "file://" + encodeURI(path) : ""
}

function compactTokenCount(value) {
    if (value >= 1000000) {
        const millions = value / 1000000
        return (millions >= 10 ? millions.toFixed(0) : millions.toFixed(1))
                .replace(/\.0$/, "") + "m"
    }
    if (value >= 1000)
        return Math.round(value / 1000) + "k"
    return value.toString()
}

function authText(status) {
    const value = status || ""
    if (value === "notLoggedIn")
        return "Not logged in"
    if (value === "unsupported")
        return "Local"
    if (value.length === 0)
        return "Unknown"
    return value.charAt(0).toUpperCase() + value.slice(1)
}

function authColor(status, neutralColor, positiveColor) {
    return status === "notLoggedIn" ? neutralColor : positiveColor
}

function modelIndex(comboBox, models, modelId) {
    const exact = comboBox.indexOfValue(modelId)
    if (exact >= 0)
        return exact
    for (let i = 0; i < models.length; ++i) {
        if (models[i].isDefault)
            return i
    }
    return models.length > 0 ? 0 : -1
}

function selectedModel(models, currentIndex) {
    if (currentIndex < 0 || currentIndex >= models.length)
        return null
    return models[currentIndex]
}

function reasoningOptions(model) {
    if (!model)
        return []
    const defaultLabel = model.defaultEffort
            ? "Default (" + model.defaultEffort + ")" : "Model default"
    const options = [{ value: "", label: defaultLabel }]
    const efforts = Array.from(model.efforts || [])
    for (let i = 0; i < efforts.length; ++i)
        options.push({ value: efforts[i],
                       label: efforts[i].charAt(0).toUpperCase() + efforts[i].slice(1) })
    return options
}

function reasoningIndex(model, reasoningEffort) {
    const options = reasoningOptions(model)
    for (let i = 0; i < options.length; ++i) {
        if (options[i].value === reasoningEffort)
            return i
    }
    return 0
}

function reasoningSupported(model, reasoningEffort) {
    const options = reasoningOptions(model)
    for (let i = 0; i < options.length; ++i) {
        if (options[i].value === reasoningEffort)
            return true
    }
    return false
}
