export const setLogClientAction = (message) => {
    return {
        type: 'LOGCLIENT',
        message: message,
    }
}

export const setLogServerAction = (message) => {
    return {
        type: 'LOGSERVER',
        message: message,
    }
}

export const setLogTrackerAction = (message) => {
    return{
        type: 'LOGTRACKER',
        message: message,
    }
}

let init = {
    client: null,
    server: null,
    tracker: null,
}

export const logAction = (state = init, action) => {
    switch(action.type){
        case "LOGCLIENT":
            {
                state.client = action.message;
                return state;
            }
        case "LOGSERVER":
            {
                state.server = action.message;
                return state;
            }
        case "LOGTRACKER":
            {
                state.tracker = action.message;
                return state;
            }
        default: 
            return state;
    }
}