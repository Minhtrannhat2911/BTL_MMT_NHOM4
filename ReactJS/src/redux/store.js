
import { combineReducers, createStore } from 'redux';
import { logAction } from './action';
const allReducers = combineReducers({
    logAction,
})

export const store = createStore(allReducers);